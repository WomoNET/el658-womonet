#include "fake_hal.h"

#include <string.h>

#include "hal/hal.h"

fake_hal_t fake_hal;

void fake_hal_reset(void)
{
    memset(&fake_hal, 0, sizeof fake_hal);
    memset(fake_hal.adc, 0xFF, sizeof fake_hal.adc);
}

void hal_init(void) {}

uint8_t hal_take_ticks(void) { return 0u; }

void hal_set_pwm(const uint16_t duty[PWM_CHANNEL_COUNT])
{
    memcpy(fake_hal.pwm, duty, sizeof fake_hal.pwm);
}

void hal_set_ch7(bool on) { fake_hal.ch7 = on; }

void hal_set_module_power(bool on) { fake_hal.module_power = on; }

void hal_select_button_input(uint8_t input) { fake_hal.selected_input = input; }

uint8_t hal_read_buttons(void)
{
    if (fake_hal.conversion_count < sizeof fake_hal.conversions) {
        fake_hal.conversions[fake_hal.conversion_count] = fake_hal.selected_input;
    }
    fake_hal.conversion_count++;
    return fake_hal.adc[fake_hal.selected_input];
}

uint8_t hal_irq_save(void) { return 0u; }

void hal_irq_restore(uint8_t state) { (void)state; }
