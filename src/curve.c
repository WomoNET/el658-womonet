#include "curve.h"

#include "config.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#define CURVE_PROGMEM PROGMEM
#define CURVE_READ(i) pgm_read_word(&curve_table[i])
#else
#define CURVE_PROGMEM
#define CURVE_READ(i) (curve_table[i])
#endif

#include "curve_table.h"

_Static_assert(CURVE_TOP == PWM_TOP, "curve table and PSC period differ");

uint16_t curve_duty(uint16_t level)
{
    uint8_t index = (uint8_t)(level >> 8);
    uint8_t fraction = (uint8_t)level;
    if (index == 0u) {
        return 0u;
    }
    uint16_t low = CURVE_READ(index);
    if (fraction == 0u || index == 255u) {
        return low;
    }
    uint16_t high = CURVE_READ(index + 1u);
    return low + (uint16_t)(((high - low) * fraction) >> 8);
}
