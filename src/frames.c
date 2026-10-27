#include "frames.h"

/* A 4-bit bus level n is the 8-bit level n * 17, so 0xF is full */
#define NIBBLE_SCALE 17u

#define STATUS_CH7 0x01u
#define STATUS_POWER 0x10u
#define STATUS_POWER_COPY 0x20u
#define STATUS_BUTTON 0x40u

typedef void (*apply_fn)(dimmer_t *dimmer, uint8_t ch, uint8_t value);

void frames_init_memory(bus_memory_t *memory)
{
    *memory = (bus_memory_t){0};
}

uint8_t frames_nibble(uint8_t level)
{
    if (level == LEVEL_OFF) {
        return 0u;
    }
    uint8_t nibble = (uint8_t)((level + NIBBLE_SCALE / 2u) / NIBBLE_SCALE);
    return nibble == 0u ? 1u : nibble;
}

/* A level that maps to the nibble the channel reports keeps its finer level */
static void apply_nibble(dimmer_t *dimmer, uint8_t ch, uint8_t nibble)
{
    if (nibble == frames_nibble(dimmer_level(dimmer, ch))) {
        return;
    }
    dimmer_set_level(dimmer, ch, (uint8_t)(nibble * NIBBLE_SCALE));
}

static void apply_switch(dimmer_t *dimmer, uint8_t ch, uint8_t on)
{
    if ((on != 0u) == (dimmer_level(dimmer, ch) != LEVEL_OFF)) {
        return;
    }
    dimmer_set_level(dimmer, ch, on != 0u ? LEVEL_FULL : LEVEL_OFF);
}

static void apply_level(dimmer_t *dimmer, uint8_t ch, uint8_t level)
{
    if (level == dimmer_level(dimmer, ch)) {
        return;
    }
    dimmer_set_level(dimmer, ch, level);
}

static void apply_if_changed(dimmer_t *dimmer, bus_memory_t *memory, uint8_t ch,
                             uint8_t value, apply_fn apply)
{
    if (memory->seen && memory->last[ch] == value) {
        return;
    }
    memory->last[ch] = value;
    if (dimmer_is_held(dimmer, ch)) {
        return;
    }
    apply(dimmer, ch, value);
}

/* Runs after the channels, so switching module power off wins over them */
static void apply_power(dimmer_t *dimmer, bus_memory_t *memory, uint8_t status)
{
    uint8_t power = (status & STATUS_POWER) != 0u ? 1u : 0u;
    memory->seen = true;
    if (power == memory->power) {
        return;
    }
    memory->power = power;
    if (power != 0u) {
        dimmer_latch_power(dimmer);
        return;
    }
    dimmer_all_off(dimmer);
}

void frames_apply_control(dimmer_t *dimmer, bus_memory_t *memory,
                          const uint8_t data[FRAME_CONTROL_LENGTH])
{
    apply_if_changed(dimmer, memory, 0u, data[0] >> 4, apply_nibble);
    apply_if_changed(dimmer, memory, 1u, data[0] & 0x0Fu, apply_nibble);
    apply_if_changed(dimmer, memory, 2u, data[1] >> 4, apply_nibble);
    apply_if_changed(dimmer, memory, CH4, data[1] & 0x01u, apply_switch);
    apply_if_changed(dimmer, memory, 4u, data[2] >> 4, apply_nibble);
    apply_if_changed(dimmer, memory, 5u, data[2] & 0x0Fu, apply_nibble);
    apply_if_changed(dimmer, memory, CH7, data[3] & STATUS_CH7, apply_switch);
    apply_power(dimmer, memory, data[3]);
}

void frames_apply_control_hires(dimmer_t *dimmer, bus_memory_t *memory,
                                const uint8_t data[FRAME_CONTROL_HIRES_LENGTH])
{
    for (uint8_t ch = 0; ch < PWM_CHANNEL_COUNT; ch++) {
        apply_if_changed(dimmer, memory, ch, data[ch], apply_level);
    }
    apply_if_changed(dimmer, memory, CH7, data[6] & STATUS_CH7, apply_switch);
    apply_power(dimmer, memory, data[6]);
}

static uint8_t status_byte(const dimmer_t *dimmer, bool button_held)
{
    uint8_t status = 0u;
    if (dimmer_level(dimmer, CH7) != LEVEL_OFF) {
        status |= STATUS_CH7;
    }
    if (dimmer_module_power(dimmer)) {
        status |= STATUS_POWER | STATUS_POWER_COPY;
    }
    if (button_held) {
        status |= STATUS_BUTTON;
    }
    return status;
}

static uint8_t nibbles(const dimmer_t *dimmer, uint8_t high_ch, uint8_t low_ch)
{
    uint8_t high = frames_nibble(dimmer_level(dimmer, high_ch));
    uint8_t low = frames_nibble(dimmer_level(dimmer, low_ch));
    return (uint8_t)((high << 4) | low);
}

void frames_build_info(const dimmer_t *dimmer, bool button_held,
                       uint8_t out[FRAME_INFO_LENGTH])
{
    uint8_t ch4_on = dimmer_level(dimmer, CH4) != LEVEL_OFF ? 1u : 0u;
    out[0] = nibbles(dimmer, 0u, 1u);
    out[1] = (uint8_t)((frames_nibble(dimmer_level(dimmer, 2u)) << 4) | ch4_on);
    out[2] = nibbles(dimmer, 4u, 5u);
    out[3] = status_byte(dimmer, button_held);
}

void frames_build_info_hires(const dimmer_t *dimmer, bool button_held,
                             uint8_t out[FRAME_INFO_HIRES_LENGTH])
{
    for (uint8_t ch = 0; ch < PWM_CHANNEL_COUNT; ch++) {
        out[ch] = dimmer_level(dimmer, ch);
    }
    out[6] = status_byte(dimmer, button_held);
    out[7] = 0u;
}
