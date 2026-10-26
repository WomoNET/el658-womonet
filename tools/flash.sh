#!/usr/bin/env bash
# Write <image.hex> to the dimmer's flash and verify it. Flash only.
set -euo pipefail
source "$(dirname "$0")/avr-isp.sh"

require_port "${1:-}" "<image.hex>"
port=$1
image=${2:-$REPO_DIR/build/el658.hex}

avrdude_isp "$port" -U "flash:w:$image:i"
