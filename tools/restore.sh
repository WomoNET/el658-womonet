#!/usr/bin/env bash
# Put the stock EL658 firmware back from the backup tools/read.sh made of this
# unit before its first flash. Flash only.
set -euo pipefail
source "$(dirname "$0")/avr-isp.sh"

require_port "${1:-}" "[backup dir]"
port=$1
dir=${2:-unit-backup}
image="$dir/flash.bin"

if [ ! -f "$image" ]; then
    echo "$image not found: restoring needs the backup tools/read.sh made before the first flash" >&2
    exit 1
fi

# A backup read after flashing this firmware is not the stock image; refuse it
# unless asked to write an unknown image (for example another stock version).
sha=$(sha256sum "$image" | cut -d' ' -f1)
if [ "$sha" != "$STOCK_SHA256" ] && [ "${ALLOW_UNKNOWN_IMAGE:-0}" != 1 ]; then
    echo "$image is not the known stock image (SHA-256 $sha);" \
        "set ALLOW_UNKNOWN_IMAGE=1 to write it anyway" >&2
    exit 1
fi

avrdude_isp "$port" -U "flash:w:$image:r"
