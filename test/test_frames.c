#include <string.h>

#include "frames.h"
#include "test.h"

typedef struct {
    dimmer_t dimmer;
    bus_memory_t memory;
    bus_memory_t hires_memory;
} fixture_t;

static void setup(fixture_t *f)
{
    dimmer_init(&f->dimmer);
    frames_init_memory(&f->memory);
    frames_init_memory(&f->hires_memory);
}

static void control(fixture_t *f, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
    const uint8_t data[FRAME_CONTROL_LENGTH] = {b0, b1, b2, b3, 0, 0};
    frames_apply_control(&f->dimmer, &f->memory, data);
}

static void control_hires(fixture_t *f, const uint8_t levels[6], uint8_t status)
{
    uint8_t data[FRAME_CONTROL_HIRES_LENGTH] = {0};
    memcpy(data, levels, 6);
    data[6] = status;
    frames_apply_control_hires(&f->dimmer, &f->hires_memory, data);
}

static uint8_t level(fixture_t *f, uint8_t ch) { return dimmer_level(&f->dimmer, ch); }

static void every_nibble_sets_a_dimmed_level(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0x18, 0xF1, 0x5A, 0x01);
    CHECK_EQ(level(&f, 0), 17);
    CHECK_EQ(level(&f, 1), 136);
    CHECK_EQ(level(&f, 2), 255);
    CHECK_EQ(level(&f, CH4), 255);
    CHECK_EQ(level(&f, 4), 85);
    CHECK_EQ(level(&f, 5), 170);
    CHECK_EQ(level(&f, CH7), 255);
}

static void repeated_frame_keeps_a_button_change(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0xF0, 0x00, 0x00, 0x10);
    dimmer_set_level(&f.dimmer, 0, 50);
    control(&f, 0xF0, 0x00, 0x00, 0x10);
    CHECK_EQ(level(&f, 0), 50);
}

static void changed_nibble_overrides_a_button_change(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0x80, 0x00, 0x00, 0x00);
    dimmer_set_level(&f.dimmer, 0, 50);
    control(&f, 0xF0, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 0), 255);
}

static void mirrored_level_keeps_the_exact_button_level(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0xF0, 0x00, 0x00, 0x00);
    dimmer_set_level(&f.dimmer, 0, 100);
    control(&f, (uint8_t)(frames_nibble(100) << 4), 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 0), 100);
}

static void first_frame_applies_every_channel(void)
{
    fixture_t f;
    setup(&f);
    dimmer_set_level(&f.dimmer, 1, 200);
    control(&f, 0x00, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 1), LEVEL_OFF);
}

static void held_channel_records_but_ignores_the_bus(void)
{
    fixture_t f;
    setup(&f);
    dimmer_set_held(&f.dimmer, 0x01);
    control(&f, 0xF0, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 0), LEVEL_OFF);
    CHECK_EQ(level(&f, 1), LEVEL_OFF);
    dimmer_set_held(&f.dimmer, 0x00);
    control(&f, 0xF0, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 0), LEVEL_OFF);
}

static void held_channel_does_not_block_other_channels(void)
{
    fixture_t f;
    setup(&f);
    dimmer_set_held(&f.dimmer, 0x01);
    control(&f, 0xFF, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 1), LEVEL_FULL);
}

static void channel_4_on_keeps_a_dimmed_level(void)
{
    fixture_t f;
    setup(&f);
    const uint8_t levels[6] = {0, 0, 0, 90, 0, 0};
    control_hires(&f, levels, 0x00);
    control(&f, 0x00, 0x01, 0x00, 0x00);
    CHECK_EQ(level(&f, CH4), 90);
    control(&f, 0x00, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, CH4), LEVEL_OFF);
}

static void power_bit_rising_holds_module_power(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0x00, 0x00, 0x00, 0x10);
    CHECK(f.dimmer.power_hold);
    CHECK(dimmer_module_power(&f.dimmer));
}

static void power_bit_falling_switches_everything_off(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0xFF, 0x01, 0x00, 0x11);
    control(&f, 0xFF, 0x01, 0x00, 0x01);
    CHECK_EQ(level(&f, 0), LEVEL_OFF);
    CHECK_EQ(level(&f, 1), LEVEL_OFF);
    CHECK_EQ(level(&f, CH4), LEVEL_OFF);
    CHECK_EQ(level(&f, CH7), LEVEL_OFF);
    CHECK(!f.dimmer.power_hold);
}

static void power_bit_clear_after_start_changes_nothing(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0xF0, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 0), LEVEL_FULL);
}

static void hires_frame_sets_exact_levels(void)
{
    fixture_t f;
    setup(&f);
    const uint8_t levels[6] = {1, 2, 3, 4, 254, 128};
    control_hires(&f, levels, 0x11);
    for (uint8_t ch = 0; ch < 6; ch++) {
        CHECK_EQ(level(&f, ch), levels[ch]);
    }
    CHECK_EQ(level(&f, CH7), LEVEL_FULL);
    CHECK(f.dimmer.power_hold);
}

static void frame_ids_keep_separate_memories(void)
{
    fixture_t f;
    setup(&f);
    control(&f, 0xF0, 0x00, 0x00, 0x00);
    const uint8_t levels[6] = {30, 0, 0, 0, 0, 0};
    control_hires(&f, levels, 0x00);
    control(&f, 0xF0, 0x00, 0x00, 0x00);
    CHECK_EQ(level(&f, 0), 30);
}

static void info_frame_packs_nibbles_and_status(void)
{
    fixture_t f;
    setup(&f);
    const uint8_t levels[6] = {255, 136, 0, 200, 5, 0};
    control_hires(&f, levels, 0x11);
    uint8_t info[FRAME_INFO_LENGTH];
    frames_build_info(&f.dimmer, true, info);
    const uint8_t expected[FRAME_INFO_LENGTH] = {0xF8, 0x01, 0x10, 0x71};
    CHECK_BYTES(info, expected, FRAME_INFO_LENGTH);
}

static void hires_info_frame_reports_levels(void)
{
    fixture_t f;
    setup(&f);
    const uint8_t levels[6] = {9, 8, 7, 6, 5, 4};
    control_hires(&f, levels, 0x00);
    uint8_t info[FRAME_INFO_HIRES_LENGTH];
    frames_build_info_hires(&f.dimmer, false, info);
    const uint8_t expected[FRAME_INFO_HIRES_LENGTH] = {9, 8, 7, 6, 5, 4, 0x30, 0x00};
    CHECK_BYTES(info, expected, FRAME_INFO_HIRES_LENGTH);
}

static void info_without_power_has_no_status_bits(void)
{
    fixture_t f;
    setup(&f);
    uint8_t info[FRAME_INFO_LENGTH];
    frames_build_info(&f.dimmer, false, info);
    const uint8_t expected[FRAME_INFO_LENGTH] = {0, 0, 0, 0};
    CHECK_BYTES(info, expected, FRAME_INFO_LENGTH);
}

static void nibble_rounds_and_never_hides_an_on_channel(void)
{
    CHECK_EQ(frames_nibble(0), 0);
    CHECK_EQ(frames_nibble(1), 1);
    CHECK_EQ(frames_nibble(25), 1);
    CHECK_EQ(frames_nibble(26), 2);
    CHECK_EQ(frames_nibble(136), 8);
    CHECK_EQ(frames_nibble(255), 15);
}

void test_frames_suite(void)
{
    RUN(every_nibble_sets_a_dimmed_level);
    RUN(repeated_frame_keeps_a_button_change);
    RUN(changed_nibble_overrides_a_button_change);
    RUN(mirrored_level_keeps_the_exact_button_level);
    RUN(first_frame_applies_every_channel);
    RUN(held_channel_records_but_ignores_the_bus);
    RUN(held_channel_does_not_block_other_channels);
    RUN(channel_4_on_keeps_a_dimmed_level);
    RUN(power_bit_rising_holds_module_power);
    RUN(power_bit_falling_switches_everything_off);
    RUN(power_bit_clear_after_start_changes_nothing);
    RUN(hires_frame_sets_exact_levels);
    RUN(frame_ids_keep_separate_memories);
    RUN(info_frame_packs_nibbles_and_status);
    RUN(hires_info_frame_reports_levels);
    RUN(info_without_power_has_no_status_bits);
    RUN(nibble_rounds_and_never_hides_an_on_channel);
}
