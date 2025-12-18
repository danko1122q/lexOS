#!/bin/bash
# ============================================
# File: tools/mkiso.sh - Create ISO Image
# ============================================

set -e

echo "================================================"
echo "  MiniOS v2.0 - ISO Image Builder"
echo "================================================"
echo ""

# Check if kernel exists
if [ ! -f "build/kernel.bin" ]; then
    echo "Error: kernel.bin not found!"
    echo "Please run 'make' first to build the kernel."
    exit 1
fi

# Check for grub-mkrescue
if ! command -v grub-mkrescue &> /dev/null; then
    echo "Error: grub-mkrescue not found!"
    echo "Please install: sudo apt install xorriso grub-pc-bin"
    exit 1
fi

# Create ISO directory structure
echo "[1/4] Creating ISO directory structure..."
mkdir -p build/isofiles/boot/grub

# Copy kernel
echo "[2/4] Copying kernel..."
cp build/kernel.bin build/isofiles/boot/

# Create GRUB config
echo "[3/4] Creating GRUB configuration..."
cat > build/isofiles/boot/grub/grub.cfg << 'EOF'
set timeout=3
set default=0

menuentry "MiniOS v2.0" {
    multiboot /boot/kernel.bin
    boot
}

menuentry "MiniOS v2.0 (Safe Mode)" {
    multiboot /boot/kernel.bin safe
    boot
}

menuentry "MiniOS v2.0 (Debug Mode)" {
    multiboot /boot/kernel.bin debug
    boot
}
EOF

# Create ISO
echo "[4/4] Creating ISO image..."
grub-mkrescue -o build/minios.iso build/isofiles 2>&1 | grep -v "warning:"

echo ""
echo "================================================"
echo "✅ ISO image created successfully!"
echo "================================================"
ls -lh build/minios.iso
echo ""
echo "You can now run it with:"
echo "  qemu-system-i386 -cdrom build/minios.iso -m 128M"
echo ""
