# Shared settings of the ISP scripts; sourced, not run.
#
# Only flash is ever read or written. Fuses and lock bits stay untouched:
# programming RSTDISBL or clearing SPIEN would lock out ISP.

PART=m16m1
PROGRAMMER=${AVRDUDE_PROGRAMMER:-stk500v2}
# SHA-256 of the stock image as read back raw (flash.bin of tools/read.sh)
STOCK_SHA256=946013bce716d4a8d82e879a2dbfad41e01a8f49ec3a2fa7b3c72f52de89091a
REPO_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

require_port() {
    if [ -z "${1:-}" ]; then
        echo "usage: $0 <programmer port> ${2:-}" >&2
        exit 2
    fi
}

avrdude_isp() {
    local port=$1
    shift
    avrdude -c "$PROGRAMMER" -p "$PART" -P "$port" "$@"
}
