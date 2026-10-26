#include "app.h"

#include "controller.h"
#include "dimmer.h"
#include "frames.h"
#include "hal/hal.h"

lin_slave_t app_lin;

static dimmer_t dimmer;
static controller_t controller;
static bus_memory_t control_memory;
static bus_memory_t control_hires_memory;
static uint8_t scan_input;

void app_init(void)
{
    dimmer_init(&dimmer);
    controller_init(&controller);
    frames_init_memory(&control_memory);
    frames_init_memory(&control_hires_memory);
    lin_slave_init(&app_lin);
    scan_input = 0u;
    hal_select_button_input(scan_input);
}

static bool take_frame(uint8_t id, uint8_t *data)
{
    uint8_t irq = hal_irq_save();
    bool fresh = lin_slave_take(&app_lin, id, data);
    hal_irq_restore(irq);
    return fresh;
}

void app_poll(void)
{
    uint8_t data[LIN_MAX_LENGTH];
    if (take_frame(FRAME_CONTROL_ID, data)) {
        frames_apply_control(&dimmer, &control_memory, data);
    }
    if (take_frame(FRAME_CONTROL_HIRES_ID, data)) {
        frames_apply_control_hires(&dimmer, &control_hires_memory, data);
    }
}

/* The mux settles for a whole tick before its input is converted */
static void scan_buttons(void)
{
    controller_sample(&controller, &dimmer, scan_input, hal_read_buttons());
    scan_input = (uint8_t)((scan_input + 1u) % BUTTON_INPUTS);
    hal_select_button_input(scan_input);
}

static void drive_outputs(void)
{
    uint16_t duty[PWM_CHANNEL_COUNT];
    for (uint8_t ch = 0; ch < PWM_CHANNEL_COUNT; ch++) {
        duty[ch] = dimmer_duty(&dimmer, ch);
    }
    hal_set_pwm(duty);
    hal_set_ch7(dimmer_level(&dimmer, CH7) != LEVEL_OFF);
    hal_set_module_power(dimmer_module_power(&dimmer));
}

static void publish_info(void)
{
    uint8_t info[FRAME_INFO_LENGTH];
    uint8_t info_hires[FRAME_INFO_HIRES_LENGTH];
    bool button_held = controller_button_held(&controller);
    frames_build_info(&dimmer, button_held, info);
    frames_build_info_hires(&dimmer, button_held, info_hires);

    uint8_t irq = hal_irq_save();
    lin_slave_publish(&app_lin, info, info_hires);
    hal_irq_restore(irq);
}

void app_tick(void)
{
    scan_buttons();
    controller_tick(&controller, &dimmer);
    dimmer_tick(&dimmer);
    drive_outputs();
    publish_info();
}
