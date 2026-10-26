/* Moves one channel's level towards a goal, one step per tick. */
#ifndef FADE_H
#define FADE_H

#include <stdbool.h>
#include <stdint.h>

/* Levels are 8.8 fixed point, so slow fades still move every tick. */
typedef struct {
    uint16_t pos;
    uint16_t goal;
    uint16_t step;
} fade_t;

/** Fade from the current position to `level` in `ticks` ticks (0: jump). */
void fade_to(fade_t *fade, uint8_t level, uint16_t ticks);

/** Move towards `level` at a fixed `step` per tick (8.8 fixed point). */
void fade_ramp(fade_t *fade, uint8_t level, uint16_t step);

/** Stop at the whole level nearest to the current position. */
void fade_hold(fade_t *fade);

void fade_tick(fade_t *fade);

static inline uint8_t fade_goal(const fade_t *fade) { return (uint8_t)(fade->goal >> 8); }

/** Current position rounded to a whole level */
static inline uint8_t fade_level(const fade_t *fade)
{
    return (uint8_t)((fade->pos + 0x80u) >> 8);
}

static inline bool fade_done(const fade_t *fade) { return fade->pos == fade->goal; }

#endif
