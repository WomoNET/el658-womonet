/*
 * Frame handling of the LIN slave, independent of the LIN controller.
 * lin_slave_header / _received / _error run in interrupt context;
 * lin_slave_publish and lin_slave_take must run with interrupts disabled.
 */
#ifndef LIN_SLAVE_H
#define LIN_SLAVE_H

#include <stdbool.h>
#include <stdint.h>

#include "diag.h"
#include "frames.h"

#define LIN_MAX_LENGTH 8u

typedef enum {
    LIN_IGNORE,
    LIN_RECEIVE,
    LIN_SEND,
} lin_action_kind_t;

/* What the controller does with the response of a header */
typedef struct {
    uint8_t kind;
    uint8_t length;
    bool classic_checksum;
} lin_action_t;

typedef struct {
    uint8_t info[FRAME_INFO_LENGTH];
    uint8_t info_hires[FRAME_INFO_HIRES_LENGTH];
    uint8_t control[FRAME_CONTROL_LENGTH];
    uint8_t control_hires[FRAME_CONTROL_HIRES_LENGTH];
    /* Bit per control frame ID received since the main loop last took it */
    uint8_t fresh;
    diag_t diag;
    /* LIN errors since start, saturating */
    uint16_t errors;
} lin_slave_t;

void lin_slave_init(lin_slave_t *lin);

/** A header arrived: decide on the response; for LIN_SEND `tx` holds its data. */
lin_action_t lin_slave_header(lin_slave_t *lin, uint8_t id, uint8_t tx[LIN_MAX_LENGTH]);

/** A response requested with LIN_RECEIVE arrived with a valid checksum. */
void lin_slave_received(lin_slave_t *lin, uint8_t id, const uint8_t *data);

/** The controller flagged an error; the frame in progress is dropped. */
void lin_slave_error(lin_slave_t *lin);

void lin_slave_publish(lin_slave_t *lin, const uint8_t info[FRAME_INFO_LENGTH],
                       const uint8_t info_hires[FRAME_INFO_HIRES_LENGTH]);

/** Copy out a control frame (0x03 or 0x28) received since the last call. */
bool lin_slave_take(lin_slave_t *lin, uint8_t id, uint8_t *out);

#endif
