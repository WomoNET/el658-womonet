/* Set-up functions of the individual AVR peripherals, called by hal_init(). */
#ifndef HAL_INTERNAL_H
#define HAL_INTERNAL_H

void hal_psc_init(void);
void hal_adc_init(void);
void hal_tick_init(void);
void hal_lin_init(void);

#endif
