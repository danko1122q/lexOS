/* ============================================
 * shell/commands/cmd_dir.c - Directory Commands
 * ============================================ */
#include "cmd_registry.h"
#include "../../lib/stdio/stdio.h"

static int cmd_pwd(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("/\n");
    return 0;
}

static int cmd_cd(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: cd <directory>\n");
        return -1;
    }
    printf("cd: filesystem not fully implemented\n");
    return 0;
}

static int cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: mkdir <directory>\n");
        return -1;
    }
    printf("mkdir: filesystem not fully implemented\n");
    return 0;
}

void register_dir_commands(void) {
    register_command("pwd", cmd_pwd, "Print working directory");
    register_command("cd", cmd_cd, "Change directory");
    register_command("mkdir", cmd_mkdir, "Create directory");
}
