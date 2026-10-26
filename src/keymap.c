#include "keymap.h"

#include "config.h"

#define KEYS_PER_INPUT 3u

#define UNUSED {KEY_UNUSED, 0u}
#define DIM(channel) {KEY_DIM, (channel) - 1u}
#define SWITCH(channel) {KEY_SWITCH, (channel) - 1u}
#define ALL_OFF(channel) {KEY_SWITCH_ALL_OFF, (channel) - 1u}

/* Taken from the stock key scan; rows are inputs 0-7, columns keys 1-3 */
static const key_action_t keymap[BUTTON_INPUTS][KEYS_PER_INPUT] = {
    {UNUSED, DIM(5), DIM(1)},
    {SWITCH(7), DIM(2), ALL_OFF(4)},
    {DIM(6), ALL_OFF(7), DIM(2)},
    {DIM(3), DIM(3), DIM(3)},
    {UNUSED, UNUSED, DIM(5)},
    {DIM(1), ALL_OFF(7), DIM(2)},
    {DIM(6), ALL_OFF(7), DIM(2)},
    {DIM(1), ALL_OFF(4), DIM(2)},
};

key_action_t keymap_lookup(uint8_t input, uint8_t key)
{
    if (input >= BUTTON_INPUTS || key == 0u || key > KEYS_PER_INPUT) {
        return (key_action_t)UNUSED;
    }
    return keymap[input][key - 1u];
}
