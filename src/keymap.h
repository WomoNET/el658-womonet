/* Which channel each key of each 74HC4051 input controls, as on the stock firmware. */
#ifndef KEYMAP_H
#define KEYMAP_H

#include <stdint.h>

typedef enum {
    KEY_UNUSED,
    KEY_DIM,             /* short press toggles, hold dims */
    KEY_SWITCH,          /* any press toggles */
    KEY_SWITCH_ALL_OFF,  /* toggles, a long press switches everything off */
} key_kind_t;

typedef struct {
    uint8_t kind;
    uint8_t ch;
} key_action_t;

/**
 * Action of `key` (1-3) on mux input `input` (0-7). The input number is the
 * select line state: PC5 is bit 2, PC4 bit 1, PB2 bit 0.
 */
key_action_t keymap_lookup(uint8_t input, uint8_t key);

#endif
