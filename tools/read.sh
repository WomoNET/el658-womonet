#!/usr/bin/env bash
# Read the flash of the connected dimmer into <out.hex> and tell whether it is
# the stock EL658 image. Run this before the first flash and keep the file.
set -euo pipefail
source "$(dirname "$0")/avr-isp.sh"

require_port "${1:-}" "<out.hex>"
port=$1
out=${2:-flash-read.hex}
raw=$(mktemp)
trap 'rm -f "$raw"' EXIT

avrdude_isp "$port" -U "flash:r:$out:i" -U "flash:r:$raw:r"
sha=$(sha256sum "$raw" | cut -d' ' -f1)
echo "flash SHA-256: $sha"
if [ "$sha" = "$STOCK_SHA256" ]; then
    echo "stock EL658 firmware"
else
    echo "not the stock EL658 firmware"
fi
