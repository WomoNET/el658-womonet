/* Pins of the EL658 board: switched outputs, mux select lines, interrupt control. */
#include <avr/interrupt.h>
#include <avr/io.h>

#include "hal.h"
#include "hal_internal.h"

/* Channel 7 and module power are plain outputs on port C */
#define CH7_PIN PC2
#define MODULE_POWER_PIN PC3

/* 74HC4051 select lines: input bit 2 on PC5, bit 1 on PC4, bit 0 on PB2 */
#define MUX_BIT2_PIN PC5
#define MUX_BIT1_PIN PC4
#define MUX_BIT0_PIN PB2

/*
 * Same direction setup as the stock firmware: every pin it drives is an output
 * and starts low. The LIN pins are taken over by the LIN controller.
 */
static void init_ports(void)
{
    PORTB = 0u;
    PORTC = 0u;
    PORTD = 0u;
    PORTE = 0u;
    DDRB = 0xC7u;
    DDRC = 0x3Fu;
    DDRD = 0x1Fu;
    DDRE = 0xFFu;
}

void hal_init(void)
{
    init_ports();
    hal_psc_init();
    hal_adc_init();
    hal_tick_init();
    hal_lin_init();
}

static void write_pin(volatile uint8_t *port, uint8_t pin, bool high)
{
    if (high) {
        *port |= (uint8_t)(1u << pin);
    } else {
        *port &= (uint8_t)~(1u << pin);
    }
}

void hal_set_ch7(bool on)
{
    write_pin(&PORTC, CH7_PIN, on);
}

void hal_set_module_power(bool on)
{
    write_pin(&PORTC, MODULE_POWER_PIN, on);
}

void hal_select_button_input(uint8_t input)
{
    write_pin(&PORTC, MUX_BIT2_PIN, (input & 0x04u) != 0u);
    write_pin(&PORTC, MUX_BIT1_PIN, (input & 0x02u) != 0u);
    write_pin(&PORTB, MUX_BIT0_PIN, (input & 0x01u) != 0u);
}

uint8_t hal_irq_save(void)
{
    uint8_t state = SREG;
    cli();
    return state;
}

void hal_irq_restore(uint8_t state)
{
    __asm__ volatile("" ::: "memory");
    SREG = state;
}
