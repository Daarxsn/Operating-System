#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

"$PROJECT_ROOT/scripts/iso.sh"

if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "Error: qemu-system-x86_64 is not installed."
    echo "ISO was created successfully at:"
    echo "  $PROJECT_ROOT/XyrisOS.iso"
    exit 1
fi

echo "====================================="
echo "        Starting XyrisOS"
echo "====================================="

qemu-system-x86_64     -m 512M     -cdrom "$PROJECT_ROOT/XyrisOS.iso"     -boot d     -serial stdio     -no-reboot
