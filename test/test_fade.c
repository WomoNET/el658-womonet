#include "fade.h"
#include "test.h"

static void run(fade_t *fade, unsigned ticks)
{
    for (unsigned i = 0; i < ticks; i++) {
        fade_tick(fade);
    }
}

static void fade_reaches_goal_in_its_duration(void)
{
    fade_t fade = {0};
    fade_to(&fade, 255, 400);
    run(&fade, 398);
    CHECK(!fade_done(&fade));
    run(&fade, 3);
    CHECK(fade_done(&fade));
    CHECK_EQ(fade_level(&fade), 255);
}

static void fade_down_stops_exactly_at_goal(void)
{
    fade_t fade = {0};
    fade_to(&fade, 200, 0);
    fade_to(&fade, 3, 7);
    run(&fade, 100);
    CHECK_EQ(fade.pos, 3 << 8);
}

static void fade_with_zero_duration_jumps(void)
{
    fade_t fade = {0};
    fade_to(&fade, 77, 0);
    CHECK_EQ(fade_level(&fade), 77);
    CHECK(fade_done(&fade));
}

static void short_distance_still_moves_every_tick(void)
{
    fade_t fade = {0};
    fade_to(&fade, 1, 1000);
    CHECK_EQ(fade.step, 1);
    fade_tick(&fade);
    CHECK_EQ(fade.pos, 1);
}

static void goal_is_reported_before_the_fade_arrives(void)
{
    fade_t fade = {0};
    fade_to(&fade, 128, 400);
    CHECK_EQ(fade_goal(&fade), 128);
    CHECK_EQ(fade_level(&fade), 0);
}

static void hold_stops_at_the_nearest_whole_level(void)
{
    fade_t fade = {0};
    fade_ramp(&fade, 255, 0x0140);
    run(&fade, 10);
    fade_hold(&fade);
    CHECK_EQ(fade.goal, 13 << 8);
    run(&fade, 2);
    CHECK(fade_done(&fade));
}

void test_fade_suite(void)
{
    RUN(fade_reaches_goal_in_its_duration);
    RUN(fade_down_stops_exactly_at_goal);
    RUN(fade_with_zero_duration_jumps);
    RUN(short_distance_still_moves_every_tick);
    RUN(goal_is_reported_before_the_fade_arrives);
    RUN(hold_stops_at_the_nearest_whole_level);
}
