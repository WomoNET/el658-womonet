/* ADC on the 74HC4051 common line (ADC10, PC6), configured as the stock firmware does. */
#include <avr/io.h>

#include "hal.h"
#include "hal_internal.h"

void hal_adc_init(void)
{
    /* Digital inputs off on ADC2 / ADC3, as on the stock firmware */
    DIDR0 = (uint8_t)((1u << ADC2D) | (1u << ADC3D));
    DIDR1 = (uint8_t)(1u << ADC10D);
    /* AVcc reference, left adjusted, channel ADC10 */
    ADMUX = (uint8_t)((1u << REFS0) | (1u << ADLAR) | 0x0Au);
    /* High-speed mode, AREF pin connected */
    ADCSRB = (uint8_t)((1u << ADHSM) | (1u << AREFEN));
    /* Enabled, clock / 16 = 500 kHz; 8 bits are read, so the fast clock is enough */
    ADCSRA = (uint8_t)((1u << ADEN) | (1u << ADPS2));
}

uint8_t hal_read_buttons(void)
{
    ADCSRA |= (uint8_t)(1u << ADSC);
    while ((ADCSRA & (1u << ADSC)) != 0u) {
    }
    return ADCH;
}
