#!/usr/bin/env bash
# Back up the connected dimmer into <dir>: flash, EEPROM, fuses and lock bits
# (read only), and tell whether the flash is the stock EL658 image. Run this
# before the first flash and keep the directory.
set -euo pipefail
source "$(dirname "$0")/avr-isp.sh"

require_port "${1:-}" "<backup dir>"
port=$1
dir=${2:-unit-backup}
mkdir -p "$dir"

avrdude_isp "$port" \
    -U "flash:r:$dir/flash.hex:i" -U "flash:r:$dir/flash.bin:r" \
    -U "eeprom:r:$dir/eeprom.hex:i" \
    -U "lfuse:r:$dir/lfuse.hex:h" -U "hfuse:r:$dir/hfuse.hex:h" \
    -U "efuse:r:$dir/efuse.hex:h" -U "lock:r:$dir/lock.hex:h"

sha=$(sha256sum "$dir/flash.bin" | cut -d' ' -f1)
echo "flash SHA-256: $sha"
echo "fuses: low $(cat "$dir/lfuse.hex") high $(cat "$dir/hfuse.hex")" \
    "ext $(cat "$dir/efuse.hex") lock $(cat "$dir/lock.hex")"
if [ "$sha" = "$STOCK_SHA256" ]; then
    echo "stock EL658 firmware"
else
    echo "not the stock EL658 firmware"
fi
