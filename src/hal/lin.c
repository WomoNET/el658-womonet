/*
 * LIN controller as a LIN 2.x slave at 19200 baud. Headers are detected in the
 * background; the interrupt decides per header whether to receive, send or
 * ignore the response (see lin_slave.c).
 */
#include <avr/interrupt.h>
#include <avr/io.h>

#include "../app.h"
#include "hal.h"
#include "hal_internal.h"

/* 8 MHz / (32 samples per bit * (12 + 1)) = 19231 baud, as on the stock firmware */
#define LIN_DIVIDER 12u

#define LCMD_MASK 0x07u
#define LCMD_RX_RESPONSE 0x02u
#define LCMD_TX_RESPONSE 0x03u
#define ID_MASK 0x3Fu

/* Frame whose response is being received, set when its header asked for one */
static uint8_t rx_id;
static uint8_t rx_length;

void hal_lin_init(void)
{
    LINCR = (uint8_t)(1u << LSWRES);
    LINBRR = LIN_DIVIDER;
    LINCR = (uint8_t)(1u << LENA);
    LINENIR = (uint8_t)((1u << LENERR) | (1u << LENIDOK) | (1u << LENTXOK) | (1u << LENRXOK));
}

/* The checksum model is taken from LIN13 when the response command starts */
static void start_response(uint8_t command, bool classic_checksum)
{
    uint8_t lincr = (uint8_t)(LINCR & ~(LCMD_MASK | (1u << LIN13)));
    if (classic_checksum) {
        lincr |= (uint8_t)(1u << LIN13);
    }
    LINCR = (uint8_t)(lincr | command);
}

static void handle_header(void)
{
    uint8_t data[LIN_MAX_LENGTH];
    uint8_t id = LINIDR & ID_MASK;
    lin_action_t action = lin_slave_header(&app_lin, id, data);

    if (action.kind == LIN_RECEIVE) {
        rx_id = id;
        rx_length = action.length;
        LINDLR = action.length;
        start_response(LCMD_RX_RESPONSE, action.classic_checksum);
        return;
    }
    if (action.kind == LIN_SEND) {
        LINSEL = 0u;
        for (uint8_t i = 0; i < action.length; i++) {
            LINDAT = data[i];
        }
        LINDLR = (uint8_t)(action.length << 4);
        start_response(LCMD_TX_RESPONSE, action.classic_checksum);
    }
}

static void handle_received(void)
{
    uint8_t data[LIN_MAX_LENGTH];
    LINSEL = 0u;
    for (uint8_t i = 0; i < rx_length; i++) {
        data[i] = LINDAT;
    }
    lin_slave_received(&app_lin, rx_id, data);
}

ISR(LIN_TC_vect)
{
    uint8_t status = LINSIR;
    if ((status & (1u << LIDOK)) != 0u) {
        handle_header();
        LINSIR = (uint8_t)(1u << LIDOK);
    }
    if ((status & (1u << LRXOK)) != 0u) {
        handle_received();
        LINSIR = (uint8_t)(1u << LRXOK);
    }
    if ((status & (1u << LTXOK)) != 0u) {
        LINSIR = (uint8_t)(1u << LTXOK);
    }
}

/* The controller has already dropped the frame and waits for the next header */
ISR(LIN_ERR_vect)
{
    lin_slave_error(&app_lin);
    LINSIR = (uint8_t)(1u << LERR);
}
