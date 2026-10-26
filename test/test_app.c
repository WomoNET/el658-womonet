#include "app.h"
#include "fake_hal.h"
#include "test.h"

static void setup(void)
{
    fake_hal_reset();
    app_init();
}

static void run(unsigned ms)
{
    for (unsigned i = 0; i < ms; i++) {
        app_poll();
        app_tick();
    }
}

static void receive(uint8_t id, const uint8_t *data)
{
    uint8_t tx[LIN_MAX_LENGTH];
    lin_action_t action = lin_slave_header(&app_lin, id, tx);
    CHECK_EQ(action.kind, LIN_RECEIVE);
    lin_slave_received(&app_lin, id, data);
}

static void request(uint8_t id, uint8_t *out)
{
    lin_action_t action = lin_slave_header(&app_lin, id, out);
    CHECK_EQ(action.kind, LIN_SEND);
}

static void scan_visits_every_input_in_turn(void)
{
    setup();
    run(16);
    CHECK_EQ(fake_hal.conversion_count, 16);
    for (unsigned i = 0; i < 16; i++) {
        CHECK_EQ(fake_hal.conversions[i], i % 8);
    }
}

static void control_frame_drives_the_outputs(void)
{
    setup();
    const uint8_t control[6] = {0xF0, 0x00, 0x00, 0x11, 0x00, 0x00};
    receive(0x03, control);
    run(FADE_MS + 2);
    CHECK_EQ(fake_hal.pwm[0], PWM_TOP);
    CHECK_EQ(fake_hal.pwm[1], 0);
    CHECK(fake_hal.ch7);
    CHECK(fake_hal.module_power);
}

static void info_frames_report_the_state(void)
{
    setup();
    const uint8_t control[8] = {0, 0, 0, 120, 0, 0, 0x10, 0};
    receive(0x28, control);
    run(1);
    uint8_t info[LIN_MAX_LENGTH];
    request(0x04, info);
    const uint8_t expected[4] = {0x00, 0x01, 0x00, 0x30};
    CHECK_BYTES(info, expected, 4);
    request(0x29, info);
    const uint8_t expected_hires[8] = {0, 0, 0, 120, 0, 0, 0x30, 0};
    CHECK_BYTES(info, expected_hires, 8);
}

static void button_press_is_reported_in_the_info_frame(void)
{
    setup();
    fake_hal.adc[0] = 10;
    run(100);
    uint8_t info[LIN_MAX_LENGTH];
    request(0x04, info);
    CHECK_EQ(info[3] & 0x40, 0x40);
    fake_hal.adc[0] = 0xFF;
    run(100);
    request(0x04, info);
    CHECK_EQ(info[0], 0xF0);
    CHECK_EQ(info[3] & 0x40, 0);
}

static void quiet_bus_keeps_the_outputs(void)
{
    setup();
    const uint8_t control[6] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
    receive(0x03, control);
    run(60000);
    CHECK(fake_hal.pwm[0] > 0);
}

void test_app_suite(void)
{
    RUN(scan_visits_every_input_in_turn);
    RUN(control_frame_drives_the_outputs);
    RUN(info_frames_report_the_state);
    RUN(button_press_is_reported_in_the_info_frame);
    RUN(quiet_bus_keeps_the_outputs);
}
