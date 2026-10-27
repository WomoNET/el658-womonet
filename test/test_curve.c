#include "config.h"
#include "curve.h"
#include "test.h"

static void below_level_one_is_off(void)
{
    CHECK_EQ(curve_duty(0), 0);
    CHECK_EQ(curve_duty(0x00FF), 0);
}

/* Shorter pulses leave the output MOSFET only partly on (measured on the board) */
#define SHORTEST_FULL_PULSE 9u

static void level_one_switches_fully_with_margin(void)
{
    CHECK(curve_duty(0x0100) >= SHORTEST_FULL_PULSE + SHORTEST_FULL_PULSE / 4u);
    CHECK(curve_duty(0x0100) < 2u * SHORTEST_FULL_PULSE);
}

static void full_level_is_the_pwm_top(void)
{
    CHECK_EQ(curve_duty(0xFF00), PWM_TOP);
}

static void curve_never_decreases(void)
{
    uint16_t previous = 0;
    for (uint32_t level = 0x0100; level <= 0xFF00; level++) {
        uint16_t duty = curve_duty((uint16_t)level);
        CHECK(duty >= previous);
        previous = duty;
    }
}

static void fractions_interpolate_between_levels(void)
{
    uint16_t low = curve_duty(0xC800);
    uint16_t high = curve_duty(0xC900);
    uint16_t middle = curve_duty(0xC880);
    CHECK(middle > low);
    CHECK(middle < high);
    CHECK_EQ(middle, low + (high - low) / 2);
}

static void middle_level_is_far_below_half_duty(void)
{
    /* Perceived half brightness needs about 18 % duty */
    uint16_t duty = curve_duty(0x8000);
    CHECK(duty > PWM_TOP / 8);
    CHECK(duty < PWM_TOP / 4);
}

void test_curve_suite(void)
{
    RUN(below_level_one_is_off);
    RUN(level_one_switches_fully_with_margin);
    RUN(full_level_is_the_pwm_top);
    RUN(curve_never_decreases);
    RUN(fractions_interpolate_between_levels);
    RUN(middle_level_is_far_below_half_duty);
}
