# ============================================
# File: tools/mkimg.sh - Create Raw Disk Image
# ============================================

#!/bin/bash
set -e

echo "================================================"
echo "  MiniOS v2.0 - Raw Image Builder"
echo "================================================"
echo ""

# Check if os-image.bin exists
if [ ! -f "build/os-image.bin" ]; then
    echo "Error: os-image.bin not found!"
    echo "Please run 'make img' first."
    exit 1
fi

echo "[1/2] Creating raw disk image..."
dd if=/dev/zero of=build/minios.img bs=512 count=2880 2>/dev/null

echo "[2/2] Writing bootloader and kernel..."
dd if=build/os-image.bin of=build/minios.img conv=notrunc 2>/dev/null

echo ""
echo "================================================"
echo "✅ Raw image created successfully!"
echo "================================================"
ls -lh build/minios.img
echo ""
echo "You can now run it with:"
echo "  qemu-system-i386 -drive format=raw,file=build/minios.img -m 128M"
echo ""
