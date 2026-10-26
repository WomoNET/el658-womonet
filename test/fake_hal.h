/* Recording fake of the HAL for tests of app.c. */
#ifndef FAKE_HAL_H
#define FAKE_HAL_H

#include <stdbool.h>
#include <stdint.h>

#include "config.h"

typedef struct {
    uint16_t pwm[PWM_CHANNEL_COUNT];
    bool ch7;
    bool module_power;
    uint8_t selected_input;
    /* ADC reading of each mux input */
    uint8_t adc[8];
    /* Inputs converted, in order */
    uint8_t conversions[32];
    unsigned conversion_count;
} fake_hal_t;

extern fake_hal_t fake_hal;

/** No key pressed on any input, outputs off. */
void fake_hal_reset(void);

#endif
