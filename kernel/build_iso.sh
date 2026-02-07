#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
ISO_DIR="$ROOT_DIR/iso"
OUTPUT="$BUILD_DIR/2026-os.iso"

if [[ ! -f "$BUILD_DIR/kernel.bin" ]]; then
  echo "kernel.bin not found. Run: make" >&2
  exit 1
fi

mkdir -p "$ISO_DIR/boot"
cp "$BUILD_DIR/kernel.bin" "$ISO_DIR/boot/kernel.bin"

if ! command -v grub-mkrescue >/dev/null 2>&1; then
  echo "grub-mkrescue not found. Install grub-mkrescue (grub2) and xorriso." >&2
  exit 1
fi

grub-mkrescue -o "$OUTPUT" "$ISO_DIR" >/dev/null 2>&1

echo "ISO created at: $OUTPUT"
