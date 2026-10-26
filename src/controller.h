/* Turns key events into channel actions: toggles, dimming ramps and all-off. */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

#include "buttons.h"
#include "config.h"
#include "dimmer.h"

typedef struct {
    bool active;
    uint8_t ch;
    uint16_t pause_ms;
} ramp_t;

typedef struct {
    button_t buttons[BUTTON_INPUTS];
    ramp_t ramps[BUTTON_INPUTS];
    /* Bit per input: its all-off fired during the current press */
    uint8_t all_off_fired;
    /* Bit per channel: its last ramp went up */
    uint8_t ramped_up;
} controller_t;

void controller_init(controller_t *controller);

/** Feed one ADC reading of mux input `input`; every input once per BUTTON_SAMPLE_MS. */
void controller_sample(controller_t *controller, dimmer_t *dimmer, uint8_t input, uint8_t adc);

/** Advance running ramps by one tick. */
void controller_tick(controller_t *controller, dimmer_t *dimmer);

/** Any key is held down. */
bool controller_button_held(const controller_t *controller);

#endif
