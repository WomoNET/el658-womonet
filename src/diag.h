/* LIN node identification: Read by Identifier on 0x3C / 0x3D. */
#ifndef DIAG_H
#define DIAG_H

#include <stdbool.h>
#include <stdint.h>

#define DIAG_REQUEST_ID 0x3Cu
#define DIAG_RESPONSE_ID 0x3Du
#define DIAG_FRAME_LENGTH 8u

#define DIAG_NAD 0x42u
#define DIAG_SUPPLIER_ID 0x5454u
#define DIAG_FUNCTION_ID 0x0042u
#define DIAG_VARIANT 0x01u

typedef struct {
    uint8_t response[DIAG_FRAME_LENGTH];
    bool pending;
} diag_t;

void diag_init(diag_t *diag);

/** Handle a master request frame; prepares a response when one is due. */
void diag_request(diag_t *diag, const uint8_t request[DIAG_FRAME_LENGTH]);

/**
 * Handle a slave response header: copies the pending response to `out` and
 * returns true, or returns false. Either way nothing stays pending.
 */
bool diag_response(diag_t *diag, uint8_t out[DIAG_FRAME_LENGTH]);

#endif
