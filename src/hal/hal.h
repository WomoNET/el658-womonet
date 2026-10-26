/*
 * Hardware interface of the dimmer logic. The AVR implementation lives next to
 * this header; the host tests provide a fake.
 */
#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stdint.h>

#include "../config.h"

/** Set up pins, PSC, ADC, LIN controller and the tick timer. Interrupts stay off. */
void hal_init(void);

/** Number of 1 ms ticks since the last call. */
uint8_t hal_take_ticks(void);

/** Duty (0-PWM_TOP) of channels 1-6; takes effect at the end of the PWM period. */
void hal_set_pwm(const uint16_t duty[PWM_CHANNEL_COUNT]);

void hal_set_ch7(bool on);

void hal_set_module_power(bool on);

/** Route 74HC4051 input `input` (0-7) to the ADC. */
void hal_select_button_input(uint8_t input);

/** Convert the button ADC channel; 8-bit result. */
uint8_t hal_read_buttons(void);

/** Disable interrupts; returns the state for hal_irq_restore. */
uint8_t hal_irq_save(void);

void hal_irq_restore(uint8_t state);

#endif
