#include "diag.h"
#include "test.h"

static const uint8_t positive[DIAG_FRAME_LENGTH] = {0x42, 0x06, 0xF2, 0x54, 0x54, 0x42, 0x00, 0x01};
static const uint8_t negative[DIAG_FRAME_LENGTH] = {0x42, 0x03, 0x7F, 0xB2, 0x12, 0xFF, 0xFF, 0xFF};

static void request(diag_t *diag, uint8_t nad, uint8_t pci, uint8_t sid, uint8_t id,
                    uint16_t supplier, uint16_t function)
{
    const uint8_t frame[DIAG_FRAME_LENGTH] = {
        nad, pci, sid, id, (uint8_t)supplier, (uint8_t)(supplier >> 8),
        (uint8_t)function, (uint8_t)(function >> 8),
    };
    diag_request(diag, frame);
}

static void read_product_id(diag_t *diag, uint8_t nad, uint16_t supplier, uint16_t function)
{
    request(diag, nad, 0x06, 0xB2, 0x00, supplier, function);
}

static void direct_request_gets_the_product_id(void)
{
    diag_t diag;
    diag_init(&diag);
    read_product_id(&diag, 0x42, 0x5454, 0x0042);
    uint8_t out[DIAG_FRAME_LENGTH];
    CHECK(diag_response(&diag, out));
    CHECK_BYTES(out, positive, DIAG_FRAME_LENGTH);
}

static void broadcast_request_gets_a_response_with_the_own_nad(void)
{
    diag_t diag;
    diag_init(&diag);
    read_product_id(&diag, 0x7F, 0x5454, 0x0042);
    uint8_t out[DIAG_FRAME_LENGTH];
    CHECK(diag_response(&diag, out));
    CHECK_BYTES(out, positive, DIAG_FRAME_LENGTH);
}

static void wildcards_match(void)
{
    diag_t diag;
    diag_init(&diag);
    read_product_id(&diag, 0x7F, 0x7FFF, 0xFFFF);
    uint8_t out[DIAG_FRAME_LENGTH];
    CHECK(diag_response(&diag, out));
    CHECK_BYTES(out, positive, DIAG_FRAME_LENGTH);
}

static void other_supplier_or_function_gets_no_response(void)
{
    diag_t diag;
    diag_init(&diag);
    uint8_t out[DIAG_FRAME_LENGTH];
    read_product_id(&diag, 0x7F, 0x5455, 0x0042);
    CHECK(!diag_response(&diag, out));
    read_product_id(&diag, 0x7F, 0x5454, 0x8125);
    CHECK(!diag_response(&diag, out));
}

static void other_nad_gets_no_response(void)
{
    diag_t diag;
    diag_init(&diag);
    uint8_t out[DIAG_FRAME_LENGTH];
    read_product_id(&diag, 0x01, 0x5454, 0x0042);
    CHECK(!diag_response(&diag, out));
}

static void unsupported_identifier_is_refused_when_addressed_directly(void)
{
    diag_t diag;
    diag_init(&diag);
    uint8_t out[DIAG_FRAME_LENGTH];
    request(&diag, 0x42, 0x06, 0xB2, 0x01, 0x5454, 0x0042);
    CHECK(diag_response(&diag, out));
    CHECK_BYTES(out, negative, DIAG_FRAME_LENGTH);
    request(&diag, 0x7F, 0x06, 0xB2, 0x01, 0x5454, 0x0042);
    CHECK(!diag_response(&diag, out));
}

static void other_service_or_pci_gets_no_response(void)
{
    diag_t diag;
    diag_init(&diag);
    uint8_t out[DIAG_FRAME_LENGTH];
    request(&diag, 0x42, 0x06, 0xB0, 0x00, 0x5454, 0x0042);
    CHECK(!diag_response(&diag, out));
    request(&diag, 0x42, 0x05, 0xB2, 0x00, 0x5454, 0x0042);
    CHECK(!diag_response(&diag, out));
}

static void response_is_sent_once(void)
{
    diag_t diag;
    diag_init(&diag);
    uint8_t out[DIAG_FRAME_LENGTH];
    read_product_id(&diag, 0x42, 0x5454, 0x0042);
    CHECK(diag_response(&diag, out));
    CHECK(!diag_response(&diag, out));
}

static void request_for_another_node_discards_the_pending_response(void)
{
    diag_t diag;
    diag_init(&diag);
    uint8_t out[DIAG_FRAME_LENGTH];
    read_product_id(&diag, 0x42, 0x5454, 0x0042);
    read_product_id(&diag, 0x10, 0x41DE, 0x0001);
    CHECK(!diag_response(&diag, out));
}

static void functional_request_keeps_the_pending_response(void)
{
    diag_t diag;
    diag_init(&diag);
    uint8_t out[DIAG_FRAME_LENGTH];
    read_product_id(&diag, 0x42, 0x5454, 0x0042);
    read_product_id(&diag, 0x7E, 0x5454, 0x0042);
    CHECK(diag_response(&diag, out));
}

void test_diag_suite(void)
{
    RUN(direct_request_gets_the_product_id);
    RUN(broadcast_request_gets_a_response_with_the_own_nad);
    RUN(wildcards_match);
    RUN(other_supplier_or_function_gets_no_response);
    RUN(other_nad_gets_no_response);
    RUN(unsupported_identifier_is_refused_when_addressed_directly);
    RUN(other_service_or_pci_gets_no_response);
    RUN(response_is_sent_once);
    RUN(request_for_another_node_discards_the_pending_response);
    RUN(functional_request_keeps_the_pending_response);
}
