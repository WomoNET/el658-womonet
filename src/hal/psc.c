/*
 * Power Stage Controller: channels 1-6 as six independent PWM outputs.
 *
 * One-ramp mode with CLKIO / 4 and a top of PWM_TOP gives 488 Hz at 12 bits.
 * Overlap protection is off so the A and B outputs of a module are independent.
 * An A output is on from count 0 to its duty, a B output from PWM_TOP - duty to
 * the end of the period. A channel at duty 0 is disconnected from the PSC and
 * held low by its port bit.
 */
#include <avr/io.h>

#include "hal.h"
#include "hal_internal.h"

/* Registers that start (A) or end (B) a channel's pulse, and its POC enable bit */
typedef struct {
    volatile uint8_t *high;
    volatile uint8_t *low;
    uint8_t enable;
    bool part_b;
} psc_output_t;

static const psc_output_t outputs[PWM_CHANNEL_COUNT] = {
    {&POCR0SBH, &POCR0SBL, (uint8_t)(1u << POEN0B), true},  /* channel 1, PB7 */
    {&POCR1SBH, &POCR1SBL, (uint8_t)(1u << POEN1B), true},  /* channel 2, PB6 */
    {&POCR0RAH, &POCR0RAL, (uint8_t)(1u << POEN0A), false}, /* channel 3, PD0 */
    {&POCR2SBH, &POCR2SBL, (uint8_t)(1u << POEN2B), true},  /* channel 4, PB1 */
    {&POCR2RAH, &POCR2RAL, (uint8_t)(1u << POEN2A), false}, /* channel 5, PB0 */
    {&POCR1RAH, &POCR1RAL, (uint8_t)(1u << POEN1A), false}, /* channel 6, PC0 */
};

/* 16-bit PSC registers latch on the low byte, so the high byte goes first */
static void write16(volatile uint8_t *high, volatile uint8_t *low, uint16_t value)
{
    *high = (uint8_t)(value >> 8);
    *low = (uint8_t)value;
}

void hal_psc_init(void)
{
    POC = 0u;
    PCNF = (uint8_t)((1u << POPA) | (1u << POPB));
    PMIC0 = (uint8_t)(1u << POVEN0);
    PMIC1 = (uint8_t)(1u << POVEN1);
    PMIC2 = (uint8_t)(1u << POVEN2);
    write16(&POCR_RBH, &POCR_RBL, PWM_TOP);
    write16(&POCR0SAH, &POCR0SAL, 0u);
    write16(&POCR1SAH, &POCR1SAL, 0u);
    write16(&POCR2SAH, &POCR2SAL, 0u);
    PCTL = (uint8_t)((1u << PPRE0) | (1u << PRUN));
}

/* All values and POC are taken over together at the end of the PWM period */
void hal_set_pwm(const uint16_t duty[PWM_CHANNEL_COUNT])
{
    uint8_t enable = 0u;
    PCNF |= (uint8_t)(1u << PULOCK);
    for (uint8_t ch = 0; ch < PWM_CHANNEL_COUNT; ch++) {
        const psc_output_t *out = &outputs[ch];
        uint16_t value = duty[ch] > PWM_TOP ? PWM_TOP : duty[ch];
        if (value == 0u) {
            continue;
        }
        enable |= out->enable;
        write16(out->high, out->low, out->part_b ? PWM_TOP - value : value);
    }
    POC = enable;
    PCNF &= (uint8_t)~(1u << PULOCK);
}
