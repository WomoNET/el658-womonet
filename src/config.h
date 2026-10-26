/* Channel layout and timing of the dimmer logic. One tick is 1 ms. */
#ifndef CONFIG_H
#define CONFIG_H

/* Channels 1-7 are indices 0-6; channels 1-6 run on the PSC, channel 7 is a switch. */
#define CHANNEL_COUNT 7u
#define PWM_CHANNEL_COUNT 6u
#define CH4 3u
#define CH7 6u

/* PSC period: 4096 clocks of CLKIO / 4 = 488 Hz */
#define PWM_TOP 4095u

#define LEVEL_OFF 0u
#define LEVEL_FULL 255u

/* Every switch-on, switch-off and bus level change fades in this time */
#define FADE_MS 400u
/* A button ramp runs from the lowest level to full in this time */
#define RAMP_FULL_MS 2500u
/* A held ramp waits this long at either end before it turns around */
#define RAMP_PAUSE_MS 500u
/* A button ramp never goes below this level, so holding never switches off */
#define RAMP_MIN_LEVEL 1u

/* The 74HC4051 inputs are scanned one per tick, so each is sampled every 8 ms */
#define BUTTON_INPUTS 8u
#define BUTTON_SAMPLE_MS 8u
#define BUTTON_DEBOUNCE_SAMPLES 3u
/* Released before this a press is a short press; held longer it dims */
#define BUTTON_HOLD_MS 400u
/* Held this long, an all-off key switches everything off */
#define BUTTON_ALL_OFF_MS 1200u

#endif
