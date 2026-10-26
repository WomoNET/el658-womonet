#include "lin_slave.h"
#include "test.h"

static void check_action(lin_action_t action, uint8_t kind, uint8_t length, bool classic)
{
    CHECK_EQ(action.kind, kind);
    CHECK_EQ(action.length, length);
    CHECK_EQ(action.classic_checksum, classic);
}

static void control_headers_receive_with_enhanced_checksum(void)
{
    lin_slave_t lin;
    lin_slave_init(&lin);
    uint8_t tx[LIN_MAX_LENGTH];
    check_action(lin_slave_header(&lin, 0x03, tx), LIN_RECEIVE, 6, false);
    check_action(lin_slave_header(&lin, 0x28, tx), LIN_RECEIVE, 8, false);
}

static void info_headers_send_the_published_frames(void)
{
    lin_slave_t lin;
    lin_slave_init(&lin);
    const uint8_t info[4] = {1, 2, 3, 4};
    const uint8_t info_hires[8] = {9, 8, 7, 6, 5, 4, 3, 2};
    lin_slave_publish(&lin, info, info_hires);

    uint8_t tx[LIN_MAX_LENGTH];
    check_action(lin_slave_header(&lin, 0x04, tx), LIN_SEND, 4, false);
    CHECK_BYTES(tx, info, 4);
    check_action(lin_slave_header(&lin, 0x29, tx), LIN_SEND, 8, false);
    CHECK_BYTES(tx, info_hires, 8);
}

static void diagnostic_frames_use_classic_checksum(void)
{
    lin_slave_t lin;
    lin_slave_init(&lin);
    uint8_t tx[LIN_MAX_LENGTH];
    check_action(lin_slave_header(&lin, 0x3C, tx), LIN_RECEIVE, 8, true);
    const uint8_t request[8] = {0x7F, 0x06, 0xB2, 0x00, 0x54, 0x54, 0x42, 0x00};
    lin_slave_received(&lin, 0x3C, request);
    check_action(lin_slave_header(&lin, 0x3D, tx), LIN_SEND, 8, true);
    CHECK_EQ(tx[2], 0xF2);
}

static void response_header_without_request_is_ignored(void)
{
    lin_slave_t lin;
    lin_slave_init(&lin);
    uint8_t tx[LIN_MAX_LENGTH];
    CHECK_EQ(lin_slave_header(&lin, 0x3D, tx).kind, LIN_IGNORE);
}

static void unknown_headers_are_ignored(void)
{
    lin_slave_t lin;
    lin_slave_init(&lin);
    uint8_t tx[LIN_MAX_LENGTH];
    const uint8_t ids[] = {0x00, 0x01, 0x02, 0x05, 0x22, 0x3A, 0x3E, 0x3F};
    for (unsigned i = 0; i < sizeof ids; i++) {
        CHECK_EQ(lin_slave_header(&lin, ids[i], tx).kind, LIN_IGNORE);
    }
}

static void received_control_frame_is_taken_once(void)
{
    lin_slave_t lin;
    lin_slave_init(&lin);
    const uint8_t control[6] = {0xF0, 0, 0, 0x10, 0, 0};
    lin_slave_received(&lin, 0x03, control);
    uint8_t out[LIN_MAX_LENGTH];
    CHECK(!lin_slave_take(&lin, 0x28, out));
    CHECK(lin_slave_take(&lin, 0x03, out));
    CHECK_BYTES(out, control, 6);
    CHECK(!lin_slave_take(&lin, 0x03, out));
}

static void errors_are_counted_and_saturate(void)
{
    lin_slave_t lin;
    lin_slave_init(&lin);
    lin_slave_error(&lin);
    CHECK_EQ(lin.errors, 1);
    lin.errors = UINT16_MAX;
    lin_slave_error(&lin);
    CHECK_EQ(lin.errors, UINT16_MAX);
}

void test_lin_slave_suite(void)
{
    RUN(control_headers_receive_with_enhanced_checksum);
    RUN(info_headers_send_the_published_frames);
    RUN(diagnostic_frames_use_classic_checksum);
    RUN(response_header_without_request_is_ignored);
    RUN(unknown_headers_are_ignored);
    RUN(received_control_frame_is_taken_once);
    RUN(errors_are_counted_and_saturate);
}
