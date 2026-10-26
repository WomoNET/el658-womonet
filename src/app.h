/* The dimmer application: wires buttons, bus frames and channel state to the HAL. */
#ifndef APP_H
#define APP_H

#include "lin_slave.h"

/* Global so interrupt handlers can reach it; see lin_slave.h for the access rules */
extern lin_slave_t app_lin;

void app_init(void);

/** Apply control frames received since the last call. */
void app_poll(void);

/** Run one 1 ms tick: scan a button input, advance fades, drive the outputs. */
void app_tick(void);

#endif
