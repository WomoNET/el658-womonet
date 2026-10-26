#include "buttons.h"

#include "config.h"

/* ADC thresholds of the stock key scan: higher readings mean lower key numbers */
#define ADC_NO_KEY 183u
#define ADC_KEY_1 126u
#define ADC_KEY_2 40u

#define HOLD_SAMPLES (BUTTON_HOLD_MS / BUTTON_SAMPLE_MS)
#define LONG_SAMPLES (BUTTON_ALL_OFF_MS / BUTTON_SAMPLE_MS)

_Static_assert(LONG_SAMPLES < UINT8_MAX, "held_samples saturates before the long press");

uint8_t button_decode(uint8_t adc)
{
    if (adc >= ADC_NO_KEY) {
        return 0u;
    }
    if (adc >= ADC_KEY_1) {
        return 1u;
    }
    if (adc >= ADC_KEY_2) {
        return 2u;
    }
    return 3u;
}

static button_event_t event(uint8_t type, uint8_t key)
{
    return (button_event_t){.type = type, .key = key};
}

/* A debounced change ends the current key first; a new key goes down on the next sample */
static button_event_t change_key(button_t *button)
{
    if (button->key != 0u) {
        uint8_t type = button->held_samples >= HOLD_SAMPLES ? BUTTON_RELEASE : BUTTON_SHORT;
        uint8_t key = button->key;
        button->key = 0u;
        return event(type, key);
    }
    button->key = button->candidate;
    button->held_samples = 0u;
    return event(BUTTON_PRESS, button->key);
}

button_event_t button_sample(button_t *button, uint8_t key)
{
    if (key != button->candidate) {
        button->candidate = key;
        button->stable_samples = 1u;
    } else if (button->stable_samples < BUTTON_DEBOUNCE_SAMPLES) {
        button->stable_samples++;
    }

    if (button->stable_samples >= BUTTON_DEBOUNCE_SAMPLES && button->candidate != button->key) {
        return change_key(button);
    }
    if (button->key == 0u) {
        return event(BUTTON_NONE, 0u);
    }
    if (button->held_samples < UINT8_MAX) {
        button->held_samples++;
    }
    if (button->held_samples == HOLD_SAMPLES) {
        return event(BUTTON_HOLD, button->key);
    }
    if (button->held_samples == LONG_SAMPLES) {
        return event(BUTTON_LONG, button->key);
    }
    return event(BUTTON_NONE, 0u);
}
