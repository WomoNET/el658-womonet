/* Toptron EL658 LED dimmer firmware: start-up and main loop. */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "app.h"
#include "hal/hal.h"

/*
 * A watchdog reset leaves the watchdog running with its shortest timeout, so it
 * is stopped before the C runtime initialises memory.
 */
__attribute__((naked, used, section(".init3"))) static void stop_watchdog(void)
{
    MCUSR = 0u;
    wdt_disable();
}

int main(void)
{
    hal_init();
    app_init();
    wdt_enable(WDTO_120MS);
    sei();

    for (;;) {
        wdt_reset();
        app_poll();
        for (uint8_t ticks = hal_take_ticks(); ticks > 0u; ticks--) {
            app_tick();
        }
    }
}
