/*
 * Unconditional frames: applying control frames 0x03 / 0x28 to the dimmer and
 * building info frames 0x04 / 0x29. Layouts are in PROTOCOL.md.
 */
#ifndef FRAMES_H
#define FRAMES_H

#include <stdbool.h>
#include <stdint.h>

#include "dimmer.h"

#define FRAME_CONTROL_ID 0x03u
#define FRAME_INFO_ID 0x04u
#define FRAME_CONTROL_HIRES_ID 0x28u
#define FRAME_INFO_HIRES_ID 0x29u

#define FRAME_CONTROL_LENGTH 6u
#define FRAME_INFO_LENGTH 4u
#define FRAME_CONTROL_HIRES_LENGTH 8u
#define FRAME_INFO_HIRES_LENGTH 8u

/* What one control frame ID last carried, so only changed values are applied */
typedef struct {
    uint8_t last[CHANNEL_COUNT];
    uint8_t power;
    bool seen;
} bus_memory_t;

void frames_init_memory(bus_memory_t *memory);

void frames_apply_control(dimmer_t *dimmer, bus_memory_t *memory,
                          const uint8_t data[FRAME_CONTROL_LENGTH]);

void frames_apply_control_hires(dimmer_t *dimmer, bus_memory_t *memory,
                                const uint8_t data[FRAME_CONTROL_HIRES_LENGTH]);

void frames_build_info(const dimmer_t *dimmer, bool button_held,
                       uint8_t out[FRAME_INFO_LENGTH]);

void frames_build_info_hires(const dimmer_t *dimmer, bool button_held,
                             uint8_t out[FRAME_INFO_HIRES_LENGTH]);

/** 4-bit bus level of an 8-bit level; at least 1 for any level but 0. */
uint8_t frames_nibble(uint8_t level);

#endif
