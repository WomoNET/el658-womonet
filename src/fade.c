#include "fade.h"

void fade_to(fade_t *fade, uint8_t level, uint16_t ticks)
{
    uint16_t goal = (uint16_t)level << 8;
    uint16_t distance = goal > fade->pos ? goal - fade->pos : fade->pos - goal;
    fade->goal = goal;
    if (ticks == 0u || distance == 0u) {
        fade->pos = goal;
        return;
    }
    fade->step = distance / ticks;
    if (fade->step == 0u) {
        fade->step = 1u;
    }
}

void fade_ramp(fade_t *fade, uint8_t level, uint16_t step)
{
    fade->goal = (uint16_t)level << 8;
    fade->step = step;
}

void fade_hold(fade_t *fade)
{
    fade->goal = (uint16_t)((fade->pos + 0x80u) & 0xFF00u);
}

void fade_tick(fade_t *fade)
{
    if (fade->pos < fade->goal) {
        uint16_t remaining = fade->goal - fade->pos;
        fade->pos = remaining > fade->step ? fade->pos + fade->step : fade->goal;
        return;
    }
    if (fade->pos > fade->goal) {
        uint16_t remaining = fade->pos - fade->goal;
        fade->pos = remaining > fade->step ? fade->pos - fade->step : fade->goal;
    }
}
