#include "dimmer.h"

#include "curve.h"

/* 8.8 fixed point step per tick for a ramp over the whole range */
#define RAMP_STEP ((uint16_t)(((uint32_t)LEVEL_FULL << 8) / RAMP_FULL_MS))

static uint8_t bit(uint8_t ch) { return (uint8_t)(1u << ch); }

void dimmer_init(dimmer_t *dimmer)
{
    *dimmer = (dimmer_t){0};
    for (uint8_t ch = 0; ch < PWM_CHANNEL_COUNT; ch++) {
        dimmer->remembered[ch] = LEVEL_FULL;
    }
}

void dimmer_set_level(dimmer_t *dimmer, uint8_t ch, uint8_t level)
{
    if (ch == CH7) {
        dimmer->ch7_on = level != LEVEL_OFF;
        return;
    }
    uint8_t current = dimmer_level(dimmer, ch);
    if (level == LEVEL_OFF && current != LEVEL_OFF) {
        dimmer->remembered[ch] = current;
    }
    dimmer->ramping &= (uint8_t)~bit(ch);
    fade_to(&dimmer->fade[ch], level, FADE_MS);
}

uint8_t dimmer_level(const dimmer_t *dimmer, uint8_t ch)
{
    if (ch == CH7) {
        return dimmer->ch7_on ? LEVEL_FULL : LEVEL_OFF;
    }
    if (dimmer_is_ramping(dimmer, ch)) {
        return fade_level(&dimmer->fade[ch]);
    }
    return fade_goal(&dimmer->fade[ch]);
}

void dimmer_toggle(dimmer_t *dimmer, uint8_t ch)
{
    if (dimmer_level(dimmer, ch) != LEVEL_OFF) {
        dimmer_set_level(dimmer, ch, LEVEL_OFF);
        return;
    }
    uint8_t level = ch == CH7 ? LEVEL_FULL : dimmer->remembered[ch];
    dimmer_set_level(dimmer, ch, level);
}

void dimmer_all_off(dimmer_t *dimmer)
{
    for (uint8_t ch = 0; ch < CHANNEL_COUNT; ch++) {
        dimmer_set_level(dimmer, ch, LEVEL_OFF);
    }
    dimmer->power_hold = false;
}

void dimmer_set_power_hold(dimmer_t *dimmer, bool on)
{
    dimmer->power_hold = on;
}

void dimmer_ramp(dimmer_t *dimmer, uint8_t ch, bool up)
{
    if (ch >= PWM_CHANNEL_COUNT) {
        return;
    }
    dimmer->ramping |= bit(ch);
    fade_ramp(&dimmer->fade[ch], up ? LEVEL_FULL : RAMP_MIN_LEVEL, RAMP_STEP);
}

void dimmer_ramp_stop(dimmer_t *dimmer, uint8_t ch)
{
    if (!dimmer_is_ramping(dimmer, ch)) {
        return;
    }
    dimmer->ramping &= (uint8_t)~bit(ch);
    fade_hold(&dimmer->fade[ch]);
}

bool dimmer_is_ramping(const dimmer_t *dimmer, uint8_t ch)
{
    return (dimmer->ramping & bit(ch)) != 0u;
}

bool dimmer_ramp_at_end(const dimmer_t *dimmer, uint8_t ch)
{
    return dimmer_is_ramping(dimmer, ch) && fade_done(&dimmer->fade[ch]);
}

void dimmer_set_held(dimmer_t *dimmer, uint8_t mask)
{
    dimmer->held = mask;
}

bool dimmer_is_held(const dimmer_t *dimmer, uint8_t ch)
{
    return (dimmer->held & bit(ch)) != 0u;
}

void dimmer_tick(dimmer_t *dimmer)
{
    for (uint8_t ch = 0; ch < PWM_CHANNEL_COUNT; ch++) {
        fade_tick(&dimmer->fade[ch]);
    }
}

uint16_t dimmer_duty(const dimmer_t *dimmer, uint8_t ch)
{
    return curve_duty(dimmer->fade[ch].pos);
}

bool dimmer_module_power(const dimmer_t *dimmer)
{
    if (dimmer->power_hold || dimmer->ch7_on) {
        return true;
    }
    for (uint8_t ch = 0; ch < PWM_CHANNEL_COUNT; ch++) {
        if (dimmer->fade[ch].pos != 0u || dimmer->fade[ch].goal != 0u) {
            return true;
        }
    }
    return false;
}
