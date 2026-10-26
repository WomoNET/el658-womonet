#include "dimmer.h"
#include "test.h"

static void run(dimmer_t *dimmer, unsigned ticks)
{
    for (unsigned i = 0; i < ticks; i++) {
        dimmer_tick(dimmer);
    }
}

static void new_level_fades_in(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_set_level(&dimmer, 0, LEVEL_FULL);
    run(&dimmer, FADE_MS / 2);
    CHECK(dimmer_duty(&dimmer, 0) > 0);
    CHECK(dimmer_duty(&dimmer, 0) < PWM_TOP);
    run(&dimmer, FADE_MS / 2 + 1);
    CHECK_EQ(dimmer_duty(&dimmer, 0), PWM_TOP);
}

static void level_reports_the_target_while_fading(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_set_level(&dimmer, 2, 100);
    run(&dimmer, 10);
    CHECK_EQ(dimmer_level(&dimmer, 2), 100);
}

static void toggle_switches_on_at_full_by_default(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_toggle(&dimmer, 1);
    CHECK_EQ(dimmer_level(&dimmer, 1), LEVEL_FULL);
}

static void toggle_on_restores_the_level_from_before_off(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_set_level(&dimmer, 4, 60);
    dimmer_toggle(&dimmer, 4);
    CHECK_EQ(dimmer_level(&dimmer, 4), LEVEL_OFF);
    dimmer_toggle(&dimmer, 4);
    CHECK_EQ(dimmer_level(&dimmer, 4), 60);
}

static void channel_7_switches_without_fade(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_toggle(&dimmer, CH7);
    CHECK_EQ(dimmer_level(&dimmer, CH7), LEVEL_FULL);
    dimmer_set_level(&dimmer, CH7, LEVEL_OFF);
    CHECK_EQ(dimmer_level(&dimmer, CH7), LEVEL_OFF);
}

static void all_off_switches_every_channel_and_the_hold_off(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    for (uint8_t ch = 0; ch < CHANNEL_COUNT; ch++) {
        dimmer_set_level(&dimmer, ch, 90);
    }
    dimmer_set_power_hold(&dimmer, true);
    dimmer_all_off(&dimmer);
    for (uint8_t ch = 0; ch < CHANNEL_COUNT; ch++) {
        CHECK_EQ(dimmer_level(&dimmer, ch), LEVEL_OFF);
    }
    CHECK(!dimmer.power_hold);
}

static void module_power_follows_hold_and_channels(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    CHECK(!dimmer_module_power(&dimmer));
    dimmer_set_power_hold(&dimmer, true);
    CHECK(dimmer_module_power(&dimmer));
    dimmer_set_power_hold(&dimmer, false);
    dimmer_set_level(&dimmer, CH7, LEVEL_FULL);
    CHECK(dimmer_module_power(&dimmer));
    dimmer_set_level(&dimmer, CH7, LEVEL_OFF);
    CHECK(!dimmer_module_power(&dimmer));
}

static void module_power_stays_on_until_the_fade_out_ends(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_set_level(&dimmer, 0, 50);
    CHECK(dimmer_module_power(&dimmer));
    run(&dimmer, FADE_MS + 1);
    dimmer_set_level(&dimmer, 0, LEVEL_OFF);
    run(&dimmer, FADE_MS / 2);
    CHECK(dimmer_module_power(&dimmer));
    run(&dimmer, FADE_MS);
    CHECK(!dimmer_module_power(&dimmer));
}

static void ramp_reports_its_current_level(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_ramp(&dimmer, 0, true);
    run(&dimmer, RAMP_FULL_MS / 2);
    uint8_t level = dimmer_level(&dimmer, 0);
    CHECK(level > 120);
    CHECK(level < 135);
}

static void ramp_crosses_the_range_in_the_ramp_time(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_ramp(&dimmer, 0, true);
    run(&dimmer, RAMP_FULL_MS - 50);
    CHECK(!dimmer_ramp_at_end(&dimmer, 0));
    run(&dimmer, 100);
    CHECK(dimmer_ramp_at_end(&dimmer, 0));
}

static void ramp_down_never_switches_off(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_set_level(&dimmer, 0, 40);
    dimmer_ramp(&dimmer, 0, false);
    run(&dimmer, RAMP_FULL_MS);
    CHECK_EQ(dimmer_level(&dimmer, 0), RAMP_MIN_LEVEL);
}

static void ramp_stop_keeps_the_reached_level(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_ramp(&dimmer, 3, true);
    run(&dimmer, 500);
    uint8_t reached = dimmer_level(&dimmer, 3);
    dimmer_ramp_stop(&dimmer, 3);
    run(&dimmer, 100);
    CHECK(!dimmer_is_ramping(&dimmer, 3));
    CHECK_EQ(dimmer_level(&dimmer, 3), reached);
}

static void new_level_ends_a_ramp(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_ramp(&dimmer, 0, true);
    dimmer_set_level(&dimmer, 0, LEVEL_OFF);
    CHECK(!dimmer_is_ramping(&dimmer, 0));
    CHECK_EQ(dimmer_level(&dimmer, 0), LEVEL_OFF);
}

static void held_mask_marks_channels(void)
{
    dimmer_t dimmer;
    dimmer_init(&dimmer);
    dimmer_set_held(&dimmer, 0x41);
    CHECK(dimmer_is_held(&dimmer, 0));
    CHECK(dimmer_is_held(&dimmer, CH7));
    CHECK(!dimmer_is_held(&dimmer, 1));
}

void test_dimmer_suite(void)
{
    RUN(new_level_fades_in);
    RUN(level_reports_the_target_while_fading);
    RUN(toggle_switches_on_at_full_by_default);
    RUN(toggle_on_restores_the_level_from_before_off);
    RUN(channel_7_switches_without_fade);
    RUN(all_off_switches_every_channel_and_the_hold_off);
    RUN(module_power_follows_hold_and_channels);
    RUN(module_power_stays_on_until_the_fade_out_ends);
    RUN(ramp_reports_its_current_level);
    RUN(ramp_crosses_the_range_in_the_ramp_time);
    RUN(ramp_down_never_switches_off);
    RUN(ramp_stop_keeps_the_reached_level);
    RUN(new_level_ends_a_ramp);
    RUN(held_mask_marks_channels);
}
