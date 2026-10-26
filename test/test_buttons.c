#include "buttons.h"
#include "config.h"
#include "keymap.h"
#include "test.h"

#define HOLD_SAMPLES (BUTTON_HOLD_MS / BUTTON_SAMPLE_MS)
#define LONG_SAMPLES (BUTTON_ALL_OFF_MS / BUTTON_SAMPLE_MS)

/* Feed `count` samples of `key`; returns the last event other than NONE */
static button_event_t feed(button_t *button, uint8_t key, unsigned count)
{
    button_event_t last = {BUTTON_NONE, 0};
    for (unsigned i = 0; i < count; i++) {
        button_event_t event = button_sample(button, key);
        if (event.type != BUTTON_NONE) {
            last = event;
        }
    }
    return last;
}

static void adc_thresholds_match_the_stock_key_scan(void)
{
    CHECK_EQ(button_decode(255), 0);
    CHECK_EQ(button_decode(183), 0);
    CHECK_EQ(button_decode(182), 1);
    CHECK_EQ(button_decode(126), 1);
    CHECK_EQ(button_decode(125), 2);
    CHECK_EQ(button_decode(40), 2);
    CHECK_EQ(button_decode(39), 3);
    CHECK_EQ(button_decode(0), 3);
}

static void single_glitch_is_ignored(void)
{
    button_t button = {0};
    CHECK_EQ(feed(&button, 2, BUTTON_DEBOUNCE_SAMPLES - 1).type, BUTTON_NONE);
    CHECK_EQ(feed(&button, 0, 10).type, BUTTON_NONE);
}

static void press_is_reported_after_debounce(void)
{
    button_t button = {0};
    button_event_t event = feed(&button, 2, BUTTON_DEBOUNCE_SAMPLES);
    CHECK_EQ(event.type, BUTTON_PRESS);
    CHECK_EQ(event.key, 2);
}

static void quick_release_is_a_short_press(void)
{
    button_t button = {0};
    feed(&button, 1, 10);
    button_event_t event = feed(&button, 0, BUTTON_DEBOUNCE_SAMPLES);
    CHECK_EQ(event.type, BUTTON_SHORT);
    CHECK_EQ(event.key, 1);
}

static void holding_reports_hold_then_long_then_release(void)
{
    button_t button = {0};
    feed(&button, 3, BUTTON_DEBOUNCE_SAMPLES);
    CHECK_EQ(feed(&button, 3, HOLD_SAMPLES - 1).type, BUTTON_NONE);
    CHECK_EQ(feed(&button, 3, 1).type, BUTTON_HOLD);
    CHECK_EQ(feed(&button, 3, LONG_SAMPLES - HOLD_SAMPLES - 1).type, BUTTON_NONE);
    CHECK_EQ(feed(&button, 3, 1).type, BUTTON_LONG);
    CHECK_EQ(feed(&button, 3, 300).type, BUTTON_NONE);
    CHECK_EQ(feed(&button, 0, BUTTON_DEBOUNCE_SAMPLES).type, BUTTON_RELEASE);
}

static void switching_keys_ends_the_old_press_first(void)
{
    button_t button = {0};
    feed(&button, 1, 10);
    CHECK_EQ(button_sample(&button, 2).type, BUTTON_NONE);
    CHECK_EQ(button_sample(&button, 2).type, BUTTON_NONE);
    CHECK_EQ(button_sample(&button, 2).type, BUTTON_SHORT);
    button_event_t event = button_sample(&button, 2);
    CHECK_EQ(event.type, BUTTON_PRESS);
    CHECK_EQ(event.key, 2);
}

static void keymap_follows_the_stock_firmware(void)
{
    key_action_t action = keymap_lookup(0, 3);
    CHECK_EQ(action.kind, KEY_DIM);
    CHECK_EQ(action.ch, 0);
    action = keymap_lookup(1, 3);
    CHECK_EQ(action.kind, KEY_SWITCH_ALL_OFF);
    CHECK_EQ(action.ch, CH4);
    action = keymap_lookup(1, 1);
    CHECK_EQ(action.kind, KEY_SWITCH);
    CHECK_EQ(action.ch, CH7);
    action = keymap_lookup(3, 2);
    CHECK_EQ(action.kind, KEY_DIM);
    CHECK_EQ(action.ch, 2);
    CHECK_EQ(keymap_lookup(4, 1).kind, KEY_UNUSED);
    CHECK_EQ(keymap_lookup(2, 0).kind, KEY_UNUSED);
    CHECK_EQ(keymap_lookup(8, 1).kind, KEY_UNUSED);
}

void test_buttons_suite(void)
{
    RUN(adc_thresholds_match_the_stock_key_scan);
    RUN(single_glitch_is_ignored);
    RUN(press_is_reported_after_debounce);
    RUN(quick_release_is_a_short_press);
    RUN(holding_reports_hold_then_long_then_release);
    RUN(switching_keys_ends_the_old_press_first);
    RUN(keymap_follows_the_stock_firmware);
}
