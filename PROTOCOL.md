# EL658 CI-Bus Protocol

The CI-Bus interface of this firmware for the Toptron EL658 LED dimmer. It is a LIN 2.x
slave at 19200 baud.

- **0x03 / 0x04** keep the stock EL658 layout and lengths, so a master written for the
  stock firmware (for example the Dometic display) keeps working. Unlike the stock
  firmware, every level 0x0-0xF is applied as a dimmed level.
- **0x28 / 0x29** are an additional frame pair with 8-bit levels for all six PWM channels.
- **0x3C / 0x3D** answer the LIN product identification.

Bit numbering is LSB-first: bit 0 is the least significant bit. Within a byte the high
nibble (bits 4-7) holds the lower-numbered channel.

## Frames

| ID   | Direction        | Data bytes | Checksum | Content                    |
|------|------------------|------------|----------|----------------------------|
| 0x03 | master → dimmer  | 6          | enhanced | control, 4-bit levels      |
| 0x04 | dimmer → master  | 4          | enhanced | info, 4-bit levels         |
| 0x28 | master → dimmer  | 8          | enhanced | control, 8-bit levels      |
| 0x29 | dimmer → master  | 8          | enhanced | info, 8-bit levels         |
| 0x3C | master → dimmer  | 8          | classic  | diagnostic request         |
| 0x3D | dimmer → master  | 8          | classic  | diagnostic response        |

All other frame IDs are ignored. A master may use either control frame or both; both
info frames always report the same state.

## Channels and Levels

| Channel          | Output                                  |
|------------------|-----------------------------------------|
| 1, 2, 3, 5, 6    | PWM, dimmed through a perceptual curve  |
| 4                | PWM; on/off in 0x03 / 0x04, dimmable in 0x28 / 0x29 |
| 7                | switched, no fade                       |

The channel numbers are the stock firmware's bus numbering. On the board's output
connectors they sit in swapped pairs:

| Channel   | 1  | 2  | 3  | 4  | 5  | 6  | 7  |
|-----------|----|----|----|----|----|----|----|
| Connector | K2 | K1 | K4 | K3 | K6 | K5 | K7 |

A PWM channel has 256 levels: 0 is off, 1-255 are evenly spaced in perceived brightness
(CIE lightness), 255 is full. Every change fades.

- An 8-bit level (0x28 / 0x29) is that level directly.
- A 4-bit level `n` (0x03) sets level `n × 17`, so 0xF is 255 and 0x8 is 136.
- A 4-bit level reported in 0x04 is the 8-bit level divided by 17 and rounded, but at
  least 0x1 while the channel is on.
- Channel 4 "on" in 0x03 sets level 255.

Module power is a separate switched output. As on the stock firmware it latches: it
switches on when the master sets the module power bit or when any channel is switched on
(bus or button), and stays on, also after the last channel is switched off, until an
all-off. After power-up it is off.

## Control Frame 0x03

| Byte | Bits | Function                              |
|------|------|---------------------------------------|
| 0    | 4-7  | Channel 1 level                       |
|      | 0-3  | Channel 2 level                       |
| 1    | 4-7  | Channel 3 level                       |
|      | 1-3  | Reserved, send 0                      |
|      | 0    | Channel 4 on/off                      |
| 2    | 4-7  | Channel 5 level                       |
|      | 0-3  | Channel 6 level                       |
| 3    | 5-7  | Reserved, send 0                      |
|      | 4    | Module power (see below)              |
|      | 1-3  | Reserved, send 0                      |
|      | 0    | Channel 7 on/off                      |
| 4-5  |      | Reserved, send 0                      |

## Control Frame 0x28

| Byte | Bits | Function                                         |
|------|------|--------------------------------------------------|
| 0    | 0-7  | Channel 1 level                                  |
| 1    | 0-7  | Channel 2 level                                  |
| 2    | 0-7  | Channel 3 level                                  |
| 3    | 0-7  | Channel 4 level                                  |
| 4    | 0-7  | Channel 5 level                                  |
| 5    | 0-7  | Channel 6 level                                  |
| 6    |      | Same layout as byte 3 of 0x03: bit 0 channel 7, bit 4 module power, other bits reserved (send 0) |
| 7    | 0-7  | Reserved, send 0                                 |

## Applying Control Frames

### Apply on change

- Each channel value is compared with the last value **received for that channel in the
  same frame ID**. It is applied only when it differs. A master that repeats the same
  frame therefore never undoes a change made with the buttons or with the other control
  frame.
- A button change does not update the last received values. To take over a channel the
  master sends a value that differs from the one it sent before; a master that writes the
  reported level back after a button change (as the WomoNET.core driver does) can always
  do that.
- If a 4-bit level equals the 4-bit level the channel currently reports, the channel
  keeps its exact level. A master that mirrors a button-dimmed level from 0x04 therefore
  causes no visible step.
- The first control frame of each ID after power-up applies every channel.
- There is no bus timeout and no sleep: the outputs keep their state when the bus goes
  quiet, and periodic frames are not required.
- A frame with a LIN error (checksum, framing, parity, timeout) is discarded and never
  changes an output.

### Module power (0x03 byte 3 bit 4, 0x28 byte 6 bit 4)

Applied on change only, as on the stock firmware:

- **0 → 1:** module power is switched on.
- **1 → 0:** all-off: every channel fades off and module power goes off once they are
  dark. This is applied after the channel values of the same frame, so it wins over them.

After power-up the bit counts as 0 for both frames.

### Buttons and the bus

While a button that controls a channel is held, bus values for that channel are recorded
as received but not applied: the button wins. Other channels are applied normally, and
no frames are dropped after the button is released. The result of a button change is
reported in the info frames.

A long press on an all-off key is an all-off too, like a 1 → 0 change of the module
power bit.

## Info Frame 0x04

| Byte | Bits | Function                                          |
|------|------|---------------------------------------------------|
| 0    | 4-7  | Channel 1 level                                   |
|      | 0-3  | Channel 2 level                                   |
| 1    | 4-7  | Channel 3 level                                   |
|      | 1-3  | 0                                                 |
|      | 0    | Channel 4 on                                      |
| 2    | 4-7  | Channel 5 level                                   |
|      | 0-3  | Channel 6 level                                   |
| 3    | 7    | 0                                                 |
|      | 6    | A button is held                                  |
|      | 5    | Module power output (copy of bit 4)               |
|      | 4    | Module power output                               |
|      | 1-3  | 0                                                 |
|      | 0    | Channel 7 on                                      |

## Info Frame 0x29

| Byte | Bits | Function                                         |
|------|------|--------------------------------------------------|
| 0    | 0-7  | Channel 1 level                                  |
| 1    | 0-7  | Channel 2 level                                  |
| 2    | 0-7  | Channel 3 level                                  |
| 3    | 0-7  | Channel 4 level                                  |
| 4    | 0-7  | Channel 5 level                                  |
| 5    | 0-7  | Channel 6 level                                  |
| 6    |      | Same as byte 3 of 0x04                           |
| 7    | 0-7  | 0                                                |

### Reported levels

- Levels are the **target** levels: where the channel is going, not where its fade
  currently is. While a button dims a channel, the level follows the ramp.
- Bit 5 of the status byte copies bit 4, as on the stock EL658, so masters that tell the
  EL658 from the EL753 SLD05xxx by that bit keep working.
- Bits shown as 0 are reserved and always 0.

## Diagnostics (0x3C / 0x3D)

| Property    | Value                         |
|-------------|-------------------------------|
| NAD         | 0x2A                          |
| Supplier ID | 0x5454 (Toptron GmbH)         |
| Function ID | 0x0042                        |
| Variant     | 0x01                          |

Only *Read by Identifier* (SID 0xB2) is supported.

| Request (0x3C)                             | Response (0x3D)            |
|--------------------------------------------|----------------------------|
| `2A 06 B2 00 54 54 42 00`                  | `2A 06 F2 54 54 42 00 01`  |
| `7F 06 B2 00 54 54 42 00`                  | `2A 06 F2 54 54 42 00 01`  |
| `7F 06 B2 00 FF 7F FF FF` (wildcards)      | `2A 06 F2 54 54 42 00 01`  |
| `2A 06 B2 05 54 54 42 00` (identifier ≠ 0) | `2A 03 7F B2 12 FF FF FF`  |

- The request is accepted with NAD 0x2A or the broadcast NAD 0x7F. Supplier ID 0x7FFF and
  function ID 0xFFFF are wildcards.
- The response always carries the dimmer's own NAD 0x2A.
- An unsupported identifier gets the negative response (error 0x12) only when the request
  addressed NAD 0x2A directly; a broadcast request for it gets no response.
- A request with a supplier or function ID that does not match, another NAD, another
  service or a PCI other than 0x06 gets no response.
- A response is sent on the next 0x3D header only; that header discards it in any case,
  so a later 0x3D header gets no answer. Any new 0x3C request (except one to the
  functional NAD 0x7E) discards a pending response first.
- 0x3C / 0x3D use the classic checksum; all other frames always use the enhanced
  checksum, also right after diagnostic traffic.

## Telling it apart from the stock firmware

Send the product identification request above, or request 0x29. The stock EL658 has no
diagnostics and ignores 0x29.
