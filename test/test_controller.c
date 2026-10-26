#include "controller.h"
#include "test.h"

/* Readings inside the stock thresholds for no key and keys 1-3 */
static const uint8_t key_adc[4] = {255, 150, 80, 10};

/* Mux inputs and keys from the stock key map */
#define DIM_CH1_INPUT 0u
#define DIM_CH1_KEY 3u
#define ALL_OFF_CH4_INPUT 1u
#define ALL_OFF_CH4_KEY 3u
#define SWITCH_CH7_INPUT 1u
#define SWITCH_CH7_KEY 1u
#define UNUSED_INPUT 4u
#define UNUSED_KEY 1u

typedef struct {
    controller_t controller;
    dimmer_t dimmer;
} fixture_t;

static void setup(fixture_t *f)
{
    controller_init(&f->controller);
    dimmer_init(&f->dimmer);
}

/* Hold `key` on `input` (0 releases) for `ms`, scanning one input per tick */
static void press(fixture_t *f, uint8_t input, uint8_t key, unsigned ms)
{
    for (unsigned tick = 0; tick < ms; tick++) {
        uint8_t scanned = (uint8_t)(tick % BUTTON_INPUTS);
        uint8_t adc = scanned == input ? key_adc[key] : key_adc[0];
        controller_sample(&f->controller, &f->dimmer, scanned, adc);
        controller_tick(&f->controller, &f->dimmer);
        dimmer_tick(&f->dimmer);
    }
}

static void tap(fixture_t *f, uint8_t input, uint8_t key)
{
    press(f, input, key, 100);
    press(f, input, 0, 100);
}

static uint8_t level(fixture_t *f, uint8_t ch) { return dimmer_level(&f->dimmer, ch); }

static void short_press_toggles_a_dimmable_channel(void)
{
    fixture_t f;
    setup(&f);
    tap(&f, DIM_CH1_INPUT, DIM_CH1_KEY);
    CHECK_EQ(level(&f, 0), LEVEL_FULL);
    tap(&f, DIM_CH1_INPUT, DIM_CH1_KEY);
    CHECK_EQ(level(&f, 0), LEVEL_OFF);
}

static void hold_dims_up_from_off_and_release_keeps_the_level(void)
{
    fixture_t f;
    setup(&f);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, BUTTON_HOLD_MS + 1000);
    press(&f, DIM_CH1_INPUT, 0, 100);
    uint8_t reached = level(&f, 0);
    CHECK(reached > 60);
    CHECK(reached < 140);
    press(&f, DIM_CH1_INPUT, 0, 1000);
    CHECK_EQ(level(&f, 0), reached);
}

static void next_hold_dims_the_other_way(void)
{
    fixture_t f;
    setup(&f);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, BUTTON_HOLD_MS + 1000);
    press(&f, DIM_CH1_INPUT, 0, 100);
    uint8_t first = level(&f, 0);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, BUTTON_HOLD_MS + 500);
    press(&f, DIM_CH1_INPUT, 0, 100);
    CHECK(level(&f, 0) < first);
}

static void long_hold_pauses_at_full_then_turns_around(void)
{
    fixture_t f;
    setup(&f);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, BUTTON_HOLD_MS + RAMP_FULL_MS + 100);
    CHECK_EQ(level(&f, 0), LEVEL_FULL);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, RAMP_PAUSE_MS - 200);
    CHECK_EQ(level(&f, 0), LEVEL_FULL);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, 1000);
    CHECK(level(&f, 0) < 200);
}

static void held_key_holds_its_channel_and_flags_a_button(void)
{
    fixture_t f;
    setup(&f);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, 100);
    CHECK(dimmer_is_held(&f.dimmer, 0));
    CHECK(!dimmer_is_held(&f.dimmer, 1));
    CHECK(controller_button_held(&f.controller));
    press(&f, DIM_CH1_INPUT, 0, 100);
    CHECK(!dimmer_is_held(&f.dimmer, 0));
    CHECK(!controller_button_held(&f.controller));
}

static void unused_key_still_counts_as_held(void)
{
    fixture_t f;
    setup(&f);
    press(&f, UNUSED_INPUT, UNUSED_KEY, 100);
    CHECK(controller_button_held(&f.controller));
    CHECK_EQ(f.dimmer.held, 0);
}

static void switch_key_toggles_channel_7(void)
{
    fixture_t f;
    setup(&f);
    tap(&f, SWITCH_CH7_INPUT, SWITCH_CH7_KEY);
    CHECK_EQ(level(&f, CH7), LEVEL_FULL);
    tap(&f, SWITCH_CH7_INPUT, SWITCH_CH7_KEY);
    CHECK_EQ(level(&f, CH7), LEVEL_OFF);
}

static void all_off_key_toggles_on_a_short_press(void)
{
    fixture_t f;
    setup(&f);
    tap(&f, ALL_OFF_CH4_INPUT, ALL_OFF_CH4_KEY);
    CHECK_EQ(level(&f, CH4), LEVEL_FULL);
}

static void long_press_switches_everything_off_without_toggling(void)
{
    fixture_t f;
    setup(&f);
    dimmer_set_level(&f.dimmer, 0, 200);
    dimmer_set_level(&f.dimmer, CH7, LEVEL_FULL);
    dimmer_set_power_hold(&f.dimmer, true);
    press(&f, ALL_OFF_CH4_INPUT, ALL_OFF_CH4_KEY, BUTTON_ALL_OFF_MS + 100);
    press(&f, ALL_OFF_CH4_INPUT, 0, 100);
    CHECK_EQ(level(&f, 0), LEVEL_OFF);
    CHECK_EQ(level(&f, CH4), LEVEL_OFF);
    CHECK_EQ(level(&f, CH7), LEVEL_OFF);
    CHECK(!f.dimmer.power_hold);
}

static void all_off_stops_a_running_ramp(void)
{
    fixture_t f;
    setup(&f);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, BUTTON_HOLD_MS + 500);
    dimmer_all_off(&f.dimmer);
    press(&f, DIM_CH1_INPUT, DIM_CH1_KEY, RAMP_FULL_MS + RAMP_PAUSE_MS);
    CHECK_EQ(level(&f, 0), LEVEL_OFF);
}

void test_controller_suite(void)
{
    RUN(short_press_toggles_a_dimmable_channel);
    RUN(hold_dims_up_from_off_and_release_keeps_the_level);
    RUN(next_hold_dims_the_other_way);
    RUN(long_hold_pauses_at_full_then_turns_around);
    RUN(held_key_holds_its_channel_and_flags_a_button);
    RUN(unused_key_still_counts_as_held);
    RUN(switch_key_toggles_channel_7);
    RUN(all_off_key_toggles_on_a_short_press);
    RUN(long_press_switches_everything_off_without_toggling);
    RUN(all_off_stops_a_running_ramp);
}
