/*
 * Resistor-coded keys on one 74HC4051 input: ADC decoding, debouncing and
 * press timing. One button_t per mux input; at most one of its keys counts.
 */
#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

typedef enum {
    BUTTON_NONE,
    BUTTON_PRESS,   /* a key went down */
    BUTTON_SHORT,   /* released before the hold time */
    BUTTON_HOLD,    /* held for the hold time */
    BUTTON_LONG,    /* held for the all-off time */
    BUTTON_RELEASE, /* released after the hold time */
} button_event_type_t;

typedef struct {
    uint8_t type;
    uint8_t key;
} button_event_t;

typedef struct {
    /* Debounced key, 0 for none */
    uint8_t key;
    uint8_t candidate;
    uint8_t stable_samples;
    /* Samples since the key went down, saturating */
    uint8_t held_samples;
} button_t;

/** Key 1-3 from an 8-bit (left adjusted) ADC reading, 0 for none. */
uint8_t button_decode(uint8_t adc);

/** Feed one decoded sample, taken every BUTTON_SAMPLE_MS. */
button_event_t button_sample(button_t *button, uint8_t key);

#endif
