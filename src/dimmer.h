/*
 * State of the seven channels and module power, independent of where a change
 * comes from (bus or buttons). Channel arguments are indices 0-6 for channels 1-7.
 */
#ifndef DIMMER_H
#define DIMMER_H

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "fade.h"

typedef struct {
    fade_t fade[PWM_CHANNEL_COUNT];
    /* Level a toggle switches back on to */
    uint8_t remembered[PWM_CHANNEL_COUNT];
    /* Bit per channel: a button holds it, the bus must not change it */
    uint8_t held;
    /* Bit per channel: a button ramp drives it */
    uint8_t ramping;
    bool ch7_on;
    bool power_hold;
} dimmer_t;

void dimmer_init(dimmer_t *dimmer);

/** Fade a channel to `level` (channel 7: on for any level but 0). Ends a ramp. */
void dimmer_set_level(dimmer_t *dimmer, uint8_t ch, uint8_t level);

/**
 * The level a channel reports: its fade goal, or its current position while a
 * button ramps it. Channel 7 reports LEVEL_FULL when on.
 */
uint8_t dimmer_level(const dimmer_t *dimmer, uint8_t ch);

/** Off if on, else back to the level it had when it was switched off. */
void dimmer_toggle(dimmer_t *dimmer, uint8_t ch);

/** Every channel off and the module power hold released. */
void dimmer_all_off(dimmer_t *dimmer);

void dimmer_set_power_hold(dimmer_t *dimmer, bool on);

/** Start or turn around a button ramp on a PWM channel. */
void dimmer_ramp(dimmer_t *dimmer, uint8_t ch, bool up);

/** Stop a ramp at the nearest whole level; no effect without a ramp. */
void dimmer_ramp_stop(dimmer_t *dimmer, uint8_t ch);

bool dimmer_is_ramping(const dimmer_t *dimmer, uint8_t ch);

/** The ramp has reached the end it is heading to. */
bool dimmer_ramp_at_end(const dimmer_t *dimmer, uint8_t ch);

void dimmer_set_held(dimmer_t *dimmer, uint8_t mask);

bool dimmer_is_held(const dimmer_t *dimmer, uint8_t ch);

/** Advance every fade by one tick. */
void dimmer_tick(dimmer_t *dimmer);

/** Current PSC duty of a PWM channel. */
uint16_t dimmer_duty(const dimmer_t *dimmer, uint8_t ch);

/** Module power output: held by the master, or any channel on or fading out. */
bool dimmer_module_power(const dimmer_t *dimmer);

#endif
