#include "lin_slave.h"

#include <string.h>

#define FRESH_CONTROL 0x01u
#define FRESH_CONTROL_HIRES 0x02u

void lin_slave_init(lin_slave_t *lin)
{
    *lin = (lin_slave_t){0};
    diag_init(&lin->diag);
}

static lin_action_t action(uint8_t kind, uint8_t length, bool classic_checksum)
{
    return (lin_action_t){.kind = kind, .length = length, .classic_checksum = classic_checksum};
}

lin_action_t lin_slave_header(lin_slave_t *lin, uint8_t id, uint8_t tx[LIN_MAX_LENGTH])
{
    switch (id) {
    case FRAME_CONTROL_ID:
        return action(LIN_RECEIVE, FRAME_CONTROL_LENGTH, false);
    case FRAME_CONTROL_HIRES_ID:
        return action(LIN_RECEIVE, FRAME_CONTROL_HIRES_LENGTH, false);
    case FRAME_INFO_ID:
        memcpy(tx, lin->info, FRAME_INFO_LENGTH);
        return action(LIN_SEND, FRAME_INFO_LENGTH, false);
    case FRAME_INFO_HIRES_ID:
        memcpy(tx, lin->info_hires, FRAME_INFO_HIRES_LENGTH);
        return action(LIN_SEND, FRAME_INFO_HIRES_LENGTH, false);
    case DIAG_REQUEST_ID:
        return action(LIN_RECEIVE, DIAG_FRAME_LENGTH, true);
    case DIAG_RESPONSE_ID:
        if (diag_response(&lin->diag, tx)) {
            return action(LIN_SEND, DIAG_FRAME_LENGTH, true);
        }
        return action(LIN_IGNORE, 0u, false);
    default:
        return action(LIN_IGNORE, 0u, false);
    }
}

void lin_slave_received(lin_slave_t *lin, uint8_t id, const uint8_t *data)
{
    switch (id) {
    case FRAME_CONTROL_ID:
        memcpy(lin->control, data, FRAME_CONTROL_LENGTH);
        lin->fresh |= FRESH_CONTROL;
        break;
    case FRAME_CONTROL_HIRES_ID:
        memcpy(lin->control_hires, data, FRAME_CONTROL_HIRES_LENGTH);
        lin->fresh |= FRESH_CONTROL_HIRES;
        break;
    case DIAG_REQUEST_ID:
        diag_request(&lin->diag, data);
        break;
    default:
        break;
    }
}

void lin_slave_error(lin_slave_t *lin)
{
    if (lin->errors != UINT16_MAX) {
        lin->errors++;
    }
}

void lin_slave_publish(lin_slave_t *lin, const uint8_t info[FRAME_INFO_LENGTH],
                       const uint8_t info_hires[FRAME_INFO_HIRES_LENGTH])
{
    memcpy(lin->info, info, FRAME_INFO_LENGTH);
    memcpy(lin->info_hires, info_hires, FRAME_INFO_HIRES_LENGTH);
}

bool lin_slave_take(lin_slave_t *lin, uint8_t id, uint8_t *out)
{
    if (id == FRAME_CONTROL_ID && (lin->fresh & FRESH_CONTROL) != 0u) {
        memcpy(out, lin->control, FRAME_CONTROL_LENGTH);
        lin->fresh &= (uint8_t)~FRESH_CONTROL;
        return true;
    }
    if (id == FRAME_CONTROL_HIRES_ID && (lin->fresh & FRESH_CONTROL_HIRES) != 0u) {
        memcpy(out, lin->control_hires, FRAME_CONTROL_HIRES_LENGTH);
        lin->fresh &= (uint8_t)~FRESH_CONTROL_HIRES;
        return true;
    }
    return false;
}
