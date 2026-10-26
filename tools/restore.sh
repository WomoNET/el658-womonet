#!/usr/bin/env bash
# Put the stock EL658 firmware back from the image tools/read.sh saved.
set -euo pipefail
source "$(dirname "$0")/avr-isp.sh"

require_port "${1:-}"
port=$1

# Make sure the image is the known stock image before writing it
sha=$(sha256sum "$STOCK_HEX" | cut -d' ' -f1)
if [ "$sha" != "$STOCK_HEX_SHA256" ]; then
    echo "$STOCK_HEX is not the stock image (SHA-256 $sha)" >&2
    exit 1
fi

"$(dirname "$0")/flash.sh" "$port" "$STOCK_HEX"
