#include "kernel.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "memory.h"
#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/serial/serial.h"
#include "../drivers/rtc/rtc.h"
#include "../fs/vfs/vfs.h"
#include "../fs/ramfs/ramfs.h"
#include "../fs/devfs/devfs.h"
#include "../shell/shell.h"
#include "../lib/stdio/stdio.h"

#define MULTIBOOT_MAGIC 0x2BADB002

// ============================================
// CONFIGURATION - CUSTOMIZE HERE!
// ============================================
#define BOOT_DELAY_MS 6000          // Delay before shell starts (milliseconds)
#define STEP_DELAY_MS 6000           // Delay between each boot step (milliseconds)
#define SHOW_BOOT_LOGO 1            // 1 = show logo, 0 = hide logo
// LOGO_STYLE removed since we only have one logo now

void kernel_panic(const char *message) {
    cli();
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_clear();
    printf("\n\n  KERNEL PANIC!\n");
    printf("  %s\n\n", message);
    printf("  System halted.\n");
    for(;;) hlt();
}

// Simple delay without timer (busy wait)
static void simple_delay(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 10000; i++) {
        __asm__ volatile("nop");
    }
}

// ============================================
// SINGLE LOGO STYLE (formerly Style 3)
// ============================================
static void show_boot_logo(void) {
    if (!SHOW_BOOT_LOGO) {
        return;  // Skip logo if disabled
    }
    
    // Only logo style available - preserved with original colors
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    printf("\n╔════════════════════════════════╗\n");
    printf("  ║                                ║\n");
    printf("  ║          L E X O S             ║\n");
    printf("  ║    Operating System v0.0.1     ║\n");
    printf("  ║                                ║\n");
    printf("  ╚════════════════════════════════╝\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("           Version %s\n", KERNEL_VERSION_STRING);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf(" \n\n");
}

// Boot step helper function with optional delay
static void boot_step(const char *message, void (*init_func)(void), bool with_delay) {
    printf("[ .. ] %s", message);
    
    if (init_func) {
        init_func();
    }
    
    if (with_delay) {
        simple_delay(STEP_DELAY_MS);
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    printf("\r[ OK ]");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf(" %s\n", message);
}

// Wrapper functions for boot steps
static void init_idt_wrapper(void) { idt_init(); }
static void init_irq_wrapper(void) { irq_init(); }
static void init_timer_wrapper(void) { timer_init(100); }
static void init_memory_wrapper(void) { memory_init(); }
static void init_keyboard_wrapper(void) { keyboard_init(); }

static void init_filesystems_wrapper(void) {
    vfs_init(); 
    ramfs_init(); 
    devfs_init();
    vfs_mkdir("/dev", 0755); 
    vfs_mkdir("/home", 0755); 
    vfs_mkdir("/tmp", 0777);
    vfs_mount("/", "ramfs", 0); 
    vfs_mount("/dev", "devfs", 0);
}

void kernel_main(uint32_t magic, uint32_t addr) {
    (void)addr;
    
    vga_init();
    vga_clear();
    
    if (magic != MULTIBOOT_MAGIC) {
        kernel_panic("Invalid multiboot magic!");
    }
    
    // Show boot logo (now simplified with preserved colors)
    show_boot_logo();
    
    // Boot header
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("Booting LexOS v%s...\n\n", KERNEL_VERSION_STRING);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Boot sequence with delays between steps
    boot_step("Initializing IDT...", init_idt_wrapper, true);
    boot_step("Initializing IRQ...", init_irq_wrapper, true);
    boot_step("Starting timer...", init_timer_wrapper, true);
    boot_step("Initializing memory...", init_memory_wrapper, true);
    boot_step("Initializing keyboard...", init_keyboard_wrapper, true);
    boot_step("Mounting filesystems...", init_filesystems_wrapper, true);
    
    // Boot complete message
    printf("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    printf("✓ Boot completed! Uptime: %d seconds\n\n", timer_get_seconds());
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Countdown before shell
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("Starting shell in ");
    
    int countdown = BOOT_DELAY_MS / 1000;  // Convert ms to seconds
    for (int i = countdown; i > 0; i--) {
        printf("%d... ", i);
        simple_delay(1000);
    }
    printf("\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Clear and start shell
    vga_clear();
    
    shell_init();
    shell_run();
    
    kernel_panic("Shell exited!");
}
