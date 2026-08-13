#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

echo "========================================="
echo "      Creating XyrisOS Bootable ISO"
echo "========================================="

if ! command -v xorriso >/dev/null 2>&1; then
    echo "Error: xorriso is not installed."
    exit 1
fi

if [ ! -f "$PROJECT_ROOT/build/kernel.elf" ]; then
    echo "Error: Kernel build not found."
    echo "Run ./scripts/build.sh first."
    exit 1
fi

if [ ! -f "$PROJECT_ROOT/boot/limine/limine.conf" ]; then
    echo "Error: Limine configuration not found."
    exit 1
fi

echo "[1/5] Preparing ISO directory..."

rm -rf "$PROJECT_ROOT/iso_root"

mkdir -p "$PROJECT_ROOT/iso_root/boot/limine"
mkdir -p "$PROJECT_ROOT/iso_root/EFI/BOOT"

echo "[2/5] Copying kernel..."

cp "$PROJECT_ROOT/build/kernel.elf" \
   "$PROJECT_ROOT/iso_root/boot/kernel.elf"

echo "[3/5] Copying Limine configuration and binaries..."

cp "$PROJECT_ROOT/boot/limine/limine.conf" \
   "$PROJECT_ROOT/iso_root/boot/limine/"

cp "$PROJECT_ROOT/limine-binary/limine-bios.sys" \
   "$PROJECT_ROOT/iso_root/boot/limine/"

cp "$PROJECT_ROOT/limine-binary/limine-bios-cd.bin" \
   "$PROJECT_ROOT/iso_root/boot/limine/"

cp "$PROJECT_ROOT/limine-binary/limine-uefi-cd.bin" \
   "$PROJECT_ROOT/iso_root/boot/limine/"

cp "$PROJECT_ROOT/limine-binary/BOOTX64.EFI" \
   "$PROJECT_ROOT/iso_root/EFI/BOOT/"

cp "$PROJECT_ROOT/limine-binary/BOOTIA32.EFI" \
   "$PROJECT_ROOT/iso_root/EFI/BOOT/"

echo "[4/5] Creating ISO..."

xorriso -as mkisofs \
    -R \
    -r \
    -J \
    -b boot/limine/limine-bios-cd.bin \
    -no-emul-boot \
    -boot-load-size 4 \
    -boot-info-table \
    -hfsplus \
    -apm-block-size 2048 \
    --efi-boot boot/limine/limine-uefi-cd.bin \
    -efi-boot-part \
    --efi-boot-image \
    --protective-msdos-label \
    "$PROJECT_ROOT/iso_root" \
    -o "$PROJECT_ROOT/XyrisOS.iso"

echo "[5/5] Installing Limine..."

"$PROJECT_ROOT/limine-binary/limine" \
    bios-install "$PROJECT_ROOT/XyrisOS.iso"

echo ""
echo "========================================="
echo "ISO Created Successfully!"
echo "========================================="