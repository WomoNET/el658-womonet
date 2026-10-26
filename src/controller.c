#include "controller.h"

#include "keymap.h"

void controller_init(controller_t *controller)
{
    *controller = (controller_t){0};
}

static uint8_t bit(uint8_t index) { return (uint8_t)(1u << index); }

/* Up from off or the bottom, down from full, otherwise the other way than last time */
static bool ramp_direction(const controller_t *controller, const dimmer_t *dimmer, uint8_t ch)
{
    uint8_t level = dimmer_level(dimmer, ch);
    if (level <= RAMP_MIN_LEVEL) {
        return true;
    }
    if (level >= LEVEL_FULL) {
        return false;
    }
    return (controller->ramped_up & bit(ch)) == 0u;
}

static void start_ramp(controller_t *controller, dimmer_t *dimmer, uint8_t input, uint8_t ch,
                       bool up)
{
    if (up) {
        controller->ramped_up |= bit(ch);
    } else {
        controller->ramped_up &= (uint8_t)~bit(ch);
    }
    controller->ramps[input] = (ramp_t){.active = true, .ch = ch, .pause_ms = 0u};
    dimmer_ramp(dimmer, ch, up);
}

static void stop_ramp(controller_t *controller, dimmer_t *dimmer, uint8_t input)
{
    ramp_t *ramp = &controller->ramps[input];
    if (!ramp->active) {
        return;
    }
    ramp->active = false;
    dimmer_ramp_stop(dimmer, ramp->ch);
}

static void handle_dim(controller_t *controller, dimmer_t *dimmer, uint8_t input,
                       button_event_t event, uint8_t ch)
{
    switch (event.type) {
    case BUTTON_SHORT:
        dimmer_toggle(dimmer, ch);
        break;
    case BUTTON_HOLD:
        start_ramp(controller, dimmer, input, ch, ramp_direction(controller, dimmer, ch));
        break;
    case BUTTON_RELEASE:
        stop_ramp(controller, dimmer, input);
        break;
    default:
        break;
    }
}

static void handle_switch(controller_t *controller, dimmer_t *dimmer, uint8_t input,
                          button_event_t event, key_action_t action)
{
    switch (event.type) {
    case BUTTON_PRESS:
        controller->all_off_fired &= (uint8_t)~bit(input);
        break;
    case BUTTON_LONG:
        if (action.kind == KEY_SWITCH_ALL_OFF) {
            controller->all_off_fired |= bit(input);
            dimmer_all_off(dimmer);
        }
        break;
    case BUTTON_SHORT:
    case BUTTON_RELEASE:
        if ((controller->all_off_fired & bit(input)) == 0u) {
            dimmer_toggle(dimmer, action.ch);
        }
        break;
    default:
        break;
    }
}

/* A channel is held while any key mapped to it is down */
static uint8_t held_channels(const controller_t *controller)
{
    uint8_t mask = 0u;
    for (uint8_t input = 0; input < BUTTON_INPUTS; input++) {
        key_action_t action = keymap_lookup(input, controller->buttons[input].key);
        if (action.kind != KEY_UNUSED) {
            mask |= bit(action.ch);
        }
    }
    return mask;
}

void controller_sample(controller_t *controller, dimmer_t *dimmer, uint8_t input, uint8_t adc)
{
    button_event_t event = button_sample(&controller->buttons[input], button_decode(adc));
    key_action_t action = keymap_lookup(input, event.key);
    if (event.type != BUTTON_NONE && action.kind == KEY_DIM) {
        handle_dim(controller, dimmer, input, event, action.ch);
    } else if (event.type != BUTTON_NONE && action.kind != KEY_UNUSED) {
        handle_switch(controller, dimmer, input, event, action);
    }
    dimmer_set_held(dimmer, held_channels(controller));
}

void controller_tick(controller_t *controller, dimmer_t *dimmer)
{
    for (uint8_t input = 0; input < BUTTON_INPUTS; input++) {
        ramp_t *ramp = &controller->ramps[input];
        if (!ramp->active) {
            continue;
        }
        /* Something else (an all-off) took the channel over */
        if (!dimmer_is_ramping(dimmer, ramp->ch)) {
            ramp->active = false;
            continue;
        }
        if (!dimmer_ramp_at_end(dimmer, ramp->ch)) {
            continue;
        }
        if (++ramp->pause_ms < RAMP_PAUSE_MS) {
            continue;
        }
        bool up = (controller->ramped_up & bit(ramp->ch)) == 0u;
        start_ramp(controller, dimmer, input, ramp->ch, up);
    }
}

bool controller_button_held(const controller_t *controller)
{
    for (uint8_t input = 0; input < BUTTON_INPUTS; input++) {
        if (controller->buttons[input].key != 0u) {
            return true;
        }
    }
    return false;
}
