# LexOS v0.0.1 - Makefile

ASM := nasm
CC := gcc
LD := ld
QEMU := qemu-system-i386

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
ISO_DIR := $(BUILD_DIR)/isofiles

CFLAGS := -m32 -ffreestanding -fno-builtin -fno-pie -nostdlib -nostdinc \
          -Wall -Wextra -O2 -fno-stack-protector \
          -Iinclude -Ikernel -Idrivers -Ifs -Ishell -Ilib

LDFLAGS := -m elf_i386 -T link.ld
ASMFLAGS := -f elf32

# Source files
KERNEL_ASM := kernel/kernel_entry.asm kernel/isr.asm
KERNEL_C := kernel/kernel.c kernel/idt.c kernel/irq.c kernel/timer.c kernel/memory.c
DRIVER_C := drivers/vga/vga.c drivers/keyboard/keyboard.c drivers/serial/serial.c drivers/rtc/rtc.c
FS_C := fs/vfs/vfs.c fs/ramfs/ramfs.c fs/devfs/devfs.c fs/simfs/simfs.c
SHELL_C := shell/shell.c
LIB_C := lib/string/string.c lib/stdio/stdio.c

# Object files
KERNEL_ASM_O := $(patsubst %.asm,$(OBJ_DIR)/%.o,$(KERNEL_ASM))
KERNEL_O := $(patsubst %.c,$(OBJ_DIR)/%.o,$(KERNEL_C))
DRIVER_O := $(patsubst %.c,$(OBJ_DIR)/%.o,$(DRIVER_C))
FS_O := $(patsubst %.c,$(OBJ_DIR)/%.o,$(FS_C))
SHELL_O := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SHELL_C))
LIB_O := $(patsubst %.c,$(OBJ_DIR)/%.o,$(LIB_C))

ALL_O := $(KERNEL_ASM_O) $(KERNEL_O) $(DRIVER_O) $(FS_O) $(SHELL_O) $(LIB_O)

.PHONY: all clean run run-vnc help

all: dirs $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
	@echo ""
	@echo "================================================"
	@echo "    Build complete! Run with: make run"
	@echo "================================================"
	@echo ""

dirs:
	@mkdir -p $(OBJ_DIR)/kernel $(OBJ_DIR)/drivers/{vga,keyboard,serial,rtc}
	@mkdir -p $(OBJ_DIR)/fs/{vfs,ramfs,devfs,simfs} $(OBJ_DIR)/shell
	@mkdir -p $(OBJ_DIR)/lib/{string,stdio}

$(OBJ_DIR)/%.o: %.asm
	@echo "[ASM] $<"
	@mkdir -p $(dir $@)
	@$(ASM) $(ASMFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.c
	@echo "[CC]  $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.elf: $(ALL_O)
	@echo "[LD]  kernel.elf"
	@$(LD) -m elf_i386 -T link.ld $(ALL_O) -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	@echo "[BIN] kernel.bin"
	@objcopy -O binary $< $@
	@echo ""
	@echo "Kernel built: $@"

# Run with GTK display (local)
run: $(BUILD_DIR)/kernel.elf
	@echo ""
	@echo "================================================"
	@echo "    Starting LexOS..."
	@echo "================================================"
	@echo ""
	@$(QEMU) -kernel $(BUILD_DIR)/kernel.elf -m 128M -display gtk,zoom-to-fit=on

# Run with VNC display (headless/Replit)
run-vnc: $(BUILD_DIR)/kernel.elf
	@echo ""
	@echo "================================================"
	@echo "    Starting LexOS (VNC mode)..."
	@echo "    Connect to VNC to see the OS"
	@echo "================================================"
	@echo ""
	@$(QEMU) -kernel $(BUILD_DIR)/kernel.elf -m 128M -display vnc=:0

# Run in curses mode (text console)
run-curses: $(BUILD_DIR)/kernel.elf
	@echo ""
	@echo "================================================"
	@echo "    Starting LexOS (Curses mode)..."
	@echo "================================================"
	@echo ""
	@$(QEMU) -kernel $(BUILD_DIR)/kernel.elf -m 128M -display curses

# ISO creation (with GRUB)
iso: $(BUILD_DIR)/kernel.bin
	@echo "Creating ISO..."
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/
	@cp grub.cfg $(ISO_DIR)/boot/grub/
	@rm -f /tmp/grub.*.tmp 2>/dev/null || true
	@grub-mkrescue --output=$(BUILD_DIR)/lexos.iso $(ISO_DIR) 2>&1 | grep -v "warning" | grep -v "locale" || true
	@if [ -f $(BUILD_DIR)/lexos.iso ]; then \
		echo "ISO created: $(BUILD_DIR)/lexos.iso"; \
		echo "Run with: qemu-system-i386 -cdrom $(BUILD_DIR)/lexos.iso -m 128M"; \
	else \
		echo "ISO creation failed"; \
		echo "Run with: make run"; \
	fi

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)
	@echo "Done"

help:
	@echo "LexOS v1.0.0 - Build System"
	@echo ""
	@echo "Commands:"
	@echo "  make          - Build kernel"
	@echo "  make run      - Run LexOS (GTK display)"
	@echo "  make run-vnc  - Run LexOS (VNC display)"
	@echo "  make iso      - Create bootable ISO"
	@echo "  make clean    - Clean build files"
	@echo ""
	@echo "Quick start: make && make run-vnc"
	@echo ""
