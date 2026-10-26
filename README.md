# Toptron EL658 Firmware

Replacement firmware for the Toptron EL658 LED dimmer (ATmega16M1). Compared with the
stock firmware it dims every channel over the CI-Bus, drives channels 1-6 with the
on-chip Power Stage Controller (12 bits, 488 Hz) instead of software PWM, fades every
change along a perceptual curve and answers the LIN product identification.

The bus interface is described in [PROTOCOL.md](PROTOCOL.md).

## Build

The toolchain comes from the flake's development shell:

```
nix develop
make          # build/el658.hex, prints flash and RAM usage
make test     # host tests of the logic
```

The perceptual curve `src/curve_table.h` is generated; regenerate it with
`make curve` (for example `make curve CURVE_ARGS="--min-duty 30"`).

## Flash

The board's 6-pin ISP header takes an STK500v2-compatible programmer (set
`AVRDUDE_PROGRAMMER` for another avrdude programmer type). The scripts read and write
**flash only**; never change the fuses or lock bits.

```
tools/read.sh <port> stock-read.hex   # first: read the chip, check it is the stock image
make flash PORT=<port>                # or: tools/flash.sh <port> build/el658.hex
tools/restore.sh <port>               # put the stock firmware back
```

`read.sh` compares the chip's flash with the stock image
(by its SHA-256); keep the file it writes.

## Layout

| Path            | Content                                                            |
|-----------------|--------------------------------------------------------------------|
| `src/`          | Logic in plain C, host-testable: fades, curve, buttons, frames, diagnostics |
| `src/hal/`      | AVR hardware layer: pins, PSC, ADC, LIN controller, tick timer     |
| `src/main.c`    | Start-up and main loop                                             |
| `test/`         | Host tests and a fake hardware layer                               |
| `tools/`        | Curve generator, ISP scripts                                       |

## Behaviour

- **PWM:** one-ramp mode, CLKIO / 4, period 4096 clocks: 488 Hz with 12-bit duty.
  The MOSFET gates are driven through 1 kΩ, so the edges take about 10-15 µs; the period
  is more than 100 times that. Level 1 is a pulse of `CURVE_MIN_DUTY` PSC clocks
  (0.5 µs each).
- **Curve:** 256 levels spaced in CIE lightness, interpolated during fades.
- **Fades:** switching on or off, and every bus level change, fades in 400 ms
  (`FADE_MS`). A button ramp runs from the lowest level to full in 2.5 s
  (`RAMP_FULL_MS`) and pauses 0.5 s at either end.
- **Buttons:** the key map, ADC thresholds and select-line order are the stock
  firmware's. A short press (under 400 ms) toggles a channel; holding a dimming key
  dims, reversing direction on each new hold. Holding an all-off key for 1.2 s switches
  everything off.
- **Remembered level:** a channel switched on by a button returns to the level it had
  when it was switched off, full after power-up. It is kept in RAM only: after a power
  loss every channel starts off anyway and the master restores the state, so EEPROM
  wear handling would buy nothing.
- **Bus:** values are applied only when they change, there is no bus timeout, and a
  frame with a LIN error never changes an output. LIN errors are counted.
- **Watchdog:** 120 ms, reset by the main loop.

## Notes on the stock firmware

- The stock firmware sets `DIDR0 = 0x0C` (digital inputs off on ADC2 / ADC3, PD5 / PD6)
  but writes `ADMUX` only once, for ADC10. It never samples ADC2 or ADC3; this firmware
  keeps the same `DIDR0` setting and does not use them either.
- The button inputs, their keys and the channels they control are listed in
  `src/keymap.c`.
