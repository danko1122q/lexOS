/* ============================================
 * shell/commands/cmd_system.c - System Commands
 * ============================================ */
#include "cmd_registry.h"
#include "../../lib/stdio/stdio.h"
#include "../../drivers/vga/vga.h"
#include "../../kernel/timer.h"
#include "../../kernel/memory.h"
#include "../../kernel/kernel.h"

static int cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    list_commands();
    return 0;
}

static int cmd_clear(int argc, char **argv) {
    (void)argc;
    (void)argv;
    vga_clear();
    return 0;
}

static int cmd_uname(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("LexOS v%s\n", KERNEL_VERSION_STRING);
    printf("Architecture: x86 (32-bit)\n");
    printf("Build Date: " __DATE__ " " __TIME__ "\n");
    return 0;
}

static int cmd_uptime(int argc, char **argv) {
    (void)argc;
    (void)argv;
    uint32_t seconds = timer_get_seconds();
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    
    printf("Uptime: %d hours, %d minutes, %d seconds\n", hours, minutes, secs);
    return 0;
}

static int cmd_free(int argc, char **argv) {
    (void)argc;
    (void)argv;
    uint32_t total, used, free_mem;
    memory_stats(&total, &used, &free_mem);
    
    printf("Memory Statistics:\n");
    printf("  Total: %d KB\n", total / 1024);
    printf("  Used:  %d KB\n", used / 1024);
    printf("  Free:  %d KB\n", free_mem / 1024);
    return 0;
}

static int cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return 0;
}

static int cmd_reboot(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("Rebooting system...\n");
    
    uint8_t temp;
    cli();
    do {
        temp = inb(0x64);
        if (temp & 1) inb(0x60);
    } while (temp & 2);
    outb(0x64, 0xFE);
    
    for(;;) hlt();
    return 0;
}

void register_system_commands(void) {
    register_command("help", cmd_help, "Display available commands");
    register_command("clear", cmd_clear, "Clear the screen");
    register_command("uname", cmd_uname, "Display system information");
    register_command("uptime", cmd_uptime, "Show system uptime");
    register_command("free", cmd_free, "Display memory usage");
    register_command("echo", cmd_echo, "Echo arguments");
    register_command("reboot", cmd_reboot, "Reboot the system");
}
