/* ============================================
 * shell/commands/cmd_file.c - File Commands (Stubs)
 * ============================================ */
#include "cmd_registry.h"
#include "../../lib/stdio/stdio.h"

static int cmd_ls(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("Directory listing:\n");
    printf("  (filesystem not fully implemented)\n");
    return 0;
}

static int cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: cat <filename>\n");
        return -1;
    }
    printf("cat: %s: filesystem not fully implemented\n", argv[1]);
    return 0;
}

static int cmd_touch(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: touch <filename>\n");
        return -1;
    }
    printf("touch: filesystem not fully implemented\n");
    return 0;
}

void register_file_commands(void) {
    register_command("ls", cmd_ls, "List directory contents");
    register_command("cat", cmd_cat, "Display file contents");
    register_command("touch", cmd_touch, "Create empty file");
}
