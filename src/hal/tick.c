/* 1 ms tick from Timer0 in CTC mode. */
#include <avr/interrupt.h>
#include <avr/io.h>

#include "hal.h"
#include "hal_internal.h"

/* 8 MHz / 64 = 125 kHz, 125 counts per ms */
#define TICK_COMPARE 124u

static volatile uint8_t ticks;

ISR(TIMER0_COMPA_vect)
{
    if (ticks != UINT8_MAX) {
        ticks++;
    }
}

void hal_tick_init(void)
{
    TCCR0A = (uint8_t)(1u << WGM01);
    OCR0A = TICK_COMPARE;
    TIMSK0 = (uint8_t)(1u << OCIE0A);
    TCCR0B = (uint8_t)((1u << CS01) | (1u << CS00));
}

uint8_t hal_take_ticks(void)
{
    uint8_t irq = hal_irq_save();
    uint8_t taken = ticks;
    ticks = 0u;
    hal_irq_restore(irq);
    return taken;
}
