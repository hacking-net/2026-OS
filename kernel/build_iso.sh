#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
ISO_DIR="$ROOT_DIR/iso"
ISO_ROOT="$BUILD_DIR/iso-root"
OUTPUT="$BUILD_DIR/2026-os.iso"
GRUB_CFG_SOURCE="$ISO_DIR/boot/grub/grub.cfg"

if [[ ! -f "$BUILD_DIR/kernel.elf" ]]; then
  echo "kernel.elf not found. Run: make" >&2
  exit 1
fi

if [[ ! -f "$GRUB_CFG_SOURCE" ]]; then
  echo "grub.cfg not found at $GRUB_CFG_SOURCE" >&2
  exit 1
fi

rm -rf "$ISO_ROOT"
mkdir -p "$ISO_ROOT/boot/grub"
cp "$BUILD_DIR/kernel.elf" "$ISO_ROOT/boot/kernel.elf"
cp "$GRUB_CFG_SOURCE" "$ISO_ROOT/boot/grub/grub.cfg"

if ! command -v grub-mkrescue >/dev/null 2>&1; then
  echo "grub-mkrescue not found. Install grub-mkrescue (grub2) and xorriso." >&2
  exit 1
fi

grub-mkrescue -o "$OUTPUT" "$ISO_ROOT" >/dev/null 2>&1

echo "ISO created at: $OUTPUT"
