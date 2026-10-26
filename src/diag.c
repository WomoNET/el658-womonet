#include "diag.h"

#include <string.h>

#define NAD_FUNCTIONAL 0x7Eu
#define NAD_BROADCAST 0x7Fu
#define SUPPLIER_WILDCARD 0x7FFFu
#define FUNCTION_WILDCARD 0xFFFFu

#define PCI_READ_BY_ID 0x06u
#define PCI_NEGATIVE 0x03u
#define SID_READ_BY_ID 0xB2u
#define RSID_NEGATIVE 0x7Fu
#define RSID_OFFSET 0x40u
#define IDENTIFIER_PRODUCT 0x00u
#define ERROR_SUBFUNCTION_NOT_SUPPORTED 0x12u
#define UNUSED_BYTE 0xFFu

void diag_init(diag_t *diag)
{
    *diag = (diag_t){0};
}

static uint16_t word(const uint8_t *lsb)
{
    return (uint16_t)(lsb[0] | (lsb[1] << 8));
}

static bool addresses_this_node(const uint8_t request[DIAG_FRAME_LENGTH])
{
    uint16_t supplier = word(&request[4]);
    uint16_t function = word(&request[6]);
    bool supplier_ok = supplier == DIAG_SUPPLIER_ID || supplier == SUPPLIER_WILDCARD;
    bool function_ok = function == DIAG_FUNCTION_ID || function == FUNCTION_WILDCARD;
    return supplier_ok && function_ok;
}

static void respond_product_id(diag_t *diag)
{
    const uint8_t response[DIAG_FRAME_LENGTH] = {
        DIAG_NAD,
        PCI_READ_BY_ID,
        SID_READ_BY_ID + RSID_OFFSET,
        (uint8_t)DIAG_SUPPLIER_ID,
        (uint8_t)(DIAG_SUPPLIER_ID >> 8),
        (uint8_t)DIAG_FUNCTION_ID,
        (uint8_t)(DIAG_FUNCTION_ID >> 8),
        DIAG_VARIANT,
    };
    memcpy(diag->response, response, sizeof response);
    diag->pending = true;
}

static void respond_negative(diag_t *diag)
{
    const uint8_t response[DIAG_FRAME_LENGTH] = {
        DIAG_NAD,        PCI_NEGATIVE, RSID_NEGATIVE, SID_READ_BY_ID,
        ERROR_SUBFUNCTION_NOT_SUPPORTED, UNUSED_BYTE, UNUSED_BYTE, UNUSED_BYTE,
    };
    memcpy(diag->response, response, sizeof response);
    diag->pending = true;
}

void diag_request(diag_t *diag, const uint8_t request[DIAG_FRAME_LENGTH])
{
    uint8_t nad = request[0];
    if (nad == NAD_FUNCTIONAL) {
        return;
    }
    diag->pending = false;
    if (nad != DIAG_NAD && nad != NAD_BROADCAST) {
        return;
    }
    if (request[1] != PCI_READ_BY_ID || request[2] != SID_READ_BY_ID) {
        return;
    }
    if (!addresses_this_node(request)) {
        return;
    }
    if (request[3] == IDENTIFIER_PRODUCT) {
        respond_product_id(diag);
        return;
    }
    if (nad == DIAG_NAD) {
        respond_negative(diag);
    }
}

bool diag_response(diag_t *diag, uint8_t out[DIAG_FRAME_LENGTH])
{
    if (!diag->pending) {
        return false;
    }
    diag->pending = false;
    memcpy(out, diag->response, DIAG_FRAME_LENGTH);
    return true;
}
