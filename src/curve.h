/* Perceptual dimming curve: level to PSC duty. */
#ifndef CURVE_H
#define CURVE_H

#include <stdint.h>

/**
 * PSC duty (0-PWM_TOP) for a level in 8.8 fixed point. Fractions interpolate
 * between neighbouring table entries so fades have no visible steps; anything
 * below level 1 is off, so the output never emits pulses shorter than level 1.
 */
uint16_t curve_duty(uint16_t level);

#endif
