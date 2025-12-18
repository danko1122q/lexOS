/* ============================================
 * shell/commands/cmd_registry.h - Command Registry
 * ============================================ */
#ifndef CMD_REGISTRY_H
#define CMD_REGISTRY_H

#define MAX_COMMANDS 64

typedef int (*command_func_t)(int argc, char **argv);

typedef struct {
    const char *name;
    command_func_t func;
    const char *description;
} command_entry_t;

void register_command(const char *name, command_func_t func, const char *desc);
int execute_command(int argc, char **argv);
void list_commands(void);

#endif
