/* ============================================
 * shell/commands/cmd_registry.c - Command Registry Implementation
 * ============================================ */
#include "cmd_registry.h"
#include "../../lib/string/string.h"
#include "../../lib/stdio/stdio.h"

static command_entry_t commands[MAX_COMMANDS];
static int command_count = 0;

void register_command(const char *name, command_func_t func, const char *desc) {
    if (command_count < MAX_COMMANDS) {
        commands[command_count].name = name;
        commands[command_count].func = func;
        commands[command_count].description = desc;
        command_count++;
    }
}

int execute_command(int argc, char **argv) {
    if (argc == 0) return -1;
    
    for (int i = 0; i < command_count; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            return commands[i].func(argc, argv);
        }
    }
    
    printf("Command not found: %s\n", argv[0]);
    printf("Type 'help' for available commands.\n");
    return -1;
}

void list_commands(void) {
    printf("\nAvailable commands:\n\n");
    for (int i = 0; i < command_count; i++) {
        printf("  %-12s - %s\n", commands[i].name, commands[i].description);
    }
    printf("\n");
}
