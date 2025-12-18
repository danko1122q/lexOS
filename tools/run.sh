# ============================================
# File: tools/run.sh - Quick Run Script
# ============================================

#!/bin/bash

if [ ! -f "build/minios.iso" ]; then
    echo "ISO not found. Building..."
    make iso
fi

echo "Starting MiniOS in QEMU..."
qemu-system-i386 -cdrom build/minios.iso \
    -m 128M \
    -display gtk,zoom-to-fit=on \
    -serial stdio
