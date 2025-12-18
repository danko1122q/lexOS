/* ================================================
 * shell/shell.c - LexOS Shell (Refactored)
 * Uses simfs for filesystem operations
 * ================================================ */
#include "shell.h"
#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/rtc/rtc.h"
#include "../lib/stdio/stdio.h"
#include "../lib/string/string.h"
#include "../kernel/timer.h"
#include "../kernel/memory.h"
#include "../kernel/kernel.h"
#include "../fs/simfs/simfs.h"

#define BUFFER_SIZE 256
#define MAX_HISTORY 10
#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80

static char input_buffer[BUFFER_SIZE];
static char history[MAX_HISTORY][BUFFER_SIZE];
static int history_count = 0;

static void itoa_custom(int num, char* str) {
    int i = 0;
    int is_negative = 0;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    while (num != 0) {
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static void print_int(int num) {
    char buffer[20];
    itoa_custom(num, buffer);
    printf("%s", buffer);
}

static void print_int_padded(int num) {
    if (num < 10) printf("0");
    print_int(num);
}

static void show_welcome(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    printf("\n+======================================+\n");
    printf("|   Welcome to LexOS Shell v0.0.1      |\n");
    printf("+======================================+\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("  Type 'help' for available commands\n\n");
}

static void show_prompt(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    printf("lexos");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf(":");
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    printf("%s", simfs_get_cwd());
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("$ ");
}

static char wait_for_key(void) {
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            if (scancode == 0x48) return 'u';
            if (scancode == 0x50) return 'd';
            if (scancode == 0x49) return 'U';
            if (scancode == 0x51) return 'D';
            if (scancode == 0x01) return 'q';
            if (scancode == 0x1C) return '\n';
            if (scancode == 0x39) return ' ';
        }
    }
}

static void cmd_help(void) {
    const char* help_lines[] = {
        "",
        "=== LexOS v0.0.1 Commands ===",
        "",
        "System Information:",
        "  help      - Show this help message",
        "  info      - Display system information",
        "  uname     - Print system name",
        "  uptime    - Show system uptime",
        "  date      - Display current date/time",
        "  free      - Display memory usage",
        "  clear     - Clear the screen",
        "",
        "File & Directory:",
        "  ls        - List files and directories",
        "  pwd       - Print working directory",
        "  cd <dir>  - Change directory",
        "  mkdir <n> - Create a new directory",
        "  rmdir <n> - Remove empty directory",
        "  touch <n> - Create a new file",
        "  cat <n>   - Display file contents",
        "  rm <n>    - Remove a file",
        "  write <n> - Simple text editor",
        "  tree      - Display directory tree",
        "",
        "Utilities:",
        "  echo <t>  - Print text to screen",
        "  calc      - Simple calculator",
        "  history   - Show command history",
        "",
        "System Control:",
        "  reboot    - Reboot the system",
        "  halt      - Halt the system",
        "",
        "Navigation: Arrow Up/Down, Page Up/Down",
        "Press ESC to exit help",
        "",
        NULL
    };
    
    int total_lines = 0;
    while (help_lines[total_lines] != NULL) total_lines++;
    
    int scroll_pos = 0;
    int max_lines = SCREEN_HEIGHT - 3;
    
    while (1) {
        vga_clear();
        
        for (int i = 0; i < max_lines && (scroll_pos + i) < total_lines; i++) {
            const char* line = help_lines[scroll_pos + i];
            
            if (line[0] == '\0') {
                printf("\n");
            } else if (line[0] == '=' || (strlen(line) > 2 && line[2] == '=')) {
                vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                printf("%s\n", line);
            } else if (line[strlen(line)-1] == ':') {
                vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
                printf("%s\n", line);
            } else {
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                printf("%s\n", line);
            }
        }
        
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("\n[Arrows/PgUp/PgDn: Scroll | ESC: Exit] ");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        printf("Line ");
        print_int(scroll_pos + 1);
        printf("-");
        print_int((scroll_pos + max_lines > total_lines) ? total_lines : scroll_pos + max_lines);
        printf(" of ");
        print_int(total_lines);
        
        char key = wait_for_key();
        
        if (key == 'q' || key == 'Q') {
            vga_clear();
            show_welcome();
            return;
        } else if (key == 'u' && scroll_pos > 0) {
            scroll_pos--;
        } else if ((key == 'd' || key == ' ' || key == '\n') && scroll_pos + max_lines < total_lines) {
            scroll_pos++;
        } else if (key == 'U') {
            scroll_pos = (scroll_pos > 5) ? scroll_pos - 5 : 0;
        } else if (key == 'D') {
            scroll_pos += 5;
            if (scroll_pos + max_lines > total_lines) {
                scroll_pos = (total_lines > max_lines) ? total_lines - max_lines : 0;
            }
        }
    }
}

static void cmd_info(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    printf("\n=== LexOS System Information ===\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    printf("OS Name:      LexOS\n");
    printf("Version:      %s\n", KERNEL_VERSION_STRING);
    printf("Architecture: x86 (32-bit)\n");
    printf("Build Date:   " __DATE__ " " __TIME__ "\n");
    
    uint32_t total, used, free_mem;
    memory_stats(&total, &used, &free_mem);
    
    printf("Memory:       ");
    print_int(total/1024);
    printf(" KB total, ");
    print_int(free_mem/1024);
    printf(" KB free\n");
    
    uint32_t sec = timer_get_seconds();
    printf("Uptime:       ");
    print_int(sec / 3600);
    printf("h ");
    print_int((sec % 3600) / 60);
    printf("m ");
    print_int(sec % 60);
    printf("s\n");
    
    printf("Shell:        LexOS Shell v0.0.1\n");
    printf("Author:       Davanico (danko1122)\n\n");
}

static void cmd_uname(void) {
    printf("LexOS v%s x86\n", KERNEL_VERSION_STRING);
}

static void cmd_uptime(void) {
    uint32_t sec = timer_get_seconds();
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    printf("System uptime: ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    print_int(sec / 3600);
    printf(" hours, ");
    print_int((sec % 3600) / 60);
    printf(" minutes, ");
    print_int(sec % 60);
    printf(" seconds\n");
}

static void cmd_date(void) {
    rtc_time_t time;
    rtc_get_time(&time);
    
    const char *months[] = {
        "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("Current Date/Time:\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("  ");
    if (time.month > 0 && time.month <= 12) {
        printf("%s ", months[time.month]);
    }
    print_int(time.day);
    printf(", ");
    print_int(time.year);
    printf(" ");
    print_int_padded(time.hour);
    printf(":");
    print_int_padded(time.minute);
    printf(":");
    print_int_padded(time.second);
    printf("\n");
}

static void cmd_free(void) {
    uint32_t total = 0, used = 0, free_mem = 0;
    memory_stats(&total, &used, &free_mem);
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("\nMemory Usage:\n");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("--------------------------------\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    printf("  Total: ");
    print_int(total / 1024);
    printf(" KB\n");
    printf("  Used:  ");
    print_int(used / 1024);
    printf(" KB\n");
    printf("  Free:  ");
    print_int(free_mem / 1024);
    printf(" KB\n");
    
    if (total > 0) {
        uint32_t used_percent = (used * 100) / total;
        printf("\nUsage: ");
        print_int(used_percent);
        printf("%%\n");
    }
}

static void cmd_ls(void) {
    char names[64][SIMFS_MAX_NAME];
    simfs_type_t types[64];
    
    int count = simfs_list_dir(NULL, names, types, 64);
    
    if (count == 0) {
        printf("(empty)\n");
        return;
    }
    
    for (int i = 0; i < count; i++) {
        if (types[i] == SIMFS_TYPE_DIR) {
            vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
            printf("%s/  ", names[i]);
        }
    }
    
    for (int i = 0; i < count; i++) {
        if (types[i] == SIMFS_TYPE_FILE) {
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            printf("%s  ", names[i]);
        }
    }
    printf("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void cmd_pwd(void) {
    printf("%s\n", simfs_get_cwd());
}

static void cmd_cd(const char *args) {
    if (args[0] == '\0') {
        simfs_set_cwd("/");
        printf("Changed to: /\n");
        return;
    }
    
    if (simfs_set_cwd(args) == 0) {
        printf("Changed to: %s\n", simfs_get_cwd());
    } else {
        printf("cd: %s: No such directory\n", args);
    }
}

static void cmd_mkdir(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: mkdir <dirname>\n");
        return;
    }
    
    int result = simfs_mkdir(args);
    if (result == 0) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("mkdir: created '%s'\n", args);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else if (result == -2) {
        printf("mkdir: cannot create directory '%s': Already exists\n", args);
    } else {
        printf("mkdir: error creating directory\n");
    }
}

static void cmd_rmdir(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: rmdir <dirname>\n");
        return;
    }
    
    int result = simfs_rmdir(args);
    if (result == 0) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("rmdir: removed '%s'\n", args);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else if (result == -2) {
        printf("rmdir: cannot remove '%s': Directory not empty\n", args);
    } else {
        printf("rmdir: cannot remove '%s': No such directory\n", args);
    }
}

static void cmd_touch(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: touch <filename>\n");
        return;
    }
    
    int result = simfs_touch(args);
    if (result == 0) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("touch: created file '%s'\n", args);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else if (result == -2) {
        printf("touch: '%s' already exists\n", args);
    } else {
        printf("touch: error creating file\n");
    }
}

static void cmd_cat(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: cat <filename>\n");
        return;
    }
    
    char buffer[SIMFS_MAX_CONTENT];
    int result = simfs_read_file(args, buffer, SIMFS_MAX_CONTENT);
    
    if (result >= 0) {
        if (buffer[0] == '\0') {
            printf("(empty file)\n");
        } else {
            printf("%s\n", buffer);
        }
    } else {
        printf("cat: %s: No such file\n", args);
    }
}

static void cmd_rm(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: rm <filename>\n");
        return;
    }
    
    if (simfs_rm(args) == 0) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("rm: removed '%s'\n", args);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else {
        printf("rm: cannot remove '%s': No such file\n", args);
    }
}

static char keyboard_scancode_to_char(uint8_t scancode) {
    static const char scancode_to_char_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_to_char_map)) {
        return scancode_to_char_map[scancode];
    }
    return 0;
}

static void cmd_write(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: write <filename>\n");
        return;
    }
    
    char filename[64] = {0};
    int j = 0;
    for (int i = 0; args[i] && args[i] != ' ' && j < 63; i++) {
        filename[j++] = args[i];
    }
    filename[j] = '\0';
    
    char existing_content[SIMFS_MAX_CONTENT] = {0};
    simfs_read_file(filename, existing_content, SIMFS_MAX_CONTENT);
    
    vga_clear();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("=== LexOS Text Editor ===\n");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("Editing: %s\n", filename);
    printf("Press ESC to save and exit\n");
    printf("Press F1 to exit without saving\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("-------------------------------------\n");
    
    char buffer[SIMFS_MAX_CONTENT];
    strcpy(buffer, existing_content);
    int pos = strlen(buffer);
    printf("%s", buffer);
    
    bool save = true;
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t sc = inb(0x60);
            
            if (sc == 0x01) { save = true; break; }
            if (sc == 0x3B) { save = false; break; }
            
            if (sc == 0x0E) {
                if (pos > 0) {
                    pos--;
                    buffer[pos] = '\0';
                    printf("\b");
                }
                continue;
            }
            
            if (sc == 0x1C) {
                if (pos < (int)(SIMFS_MAX_CONTENT - 2)) {
                    buffer[pos++] = '\n';
                    buffer[pos] = '\0';
                    printf("\n");
                }
                continue;
            }
            
            char c = keyboard_scancode_to_char(sc);
            if (c >= 32 && c <= 126 && pos < (int)(SIMFS_MAX_CONTENT - 2)) {
                buffer[pos++] = c;
                buffer[pos] = '\0';
                printf("%c", c);
            }
        }
    }
    
    if (save) {
        simfs_write_file(filename, buffer);
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("\n\nFile '%s' saved successfully!\n", filename);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        printf("\n\nFile not saved.\n");
    }
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    printf("Press any key to continue...");
    keyboard_getchar();
    vga_clear();
    show_welcome();
}

static void cmd_tree(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("%s\n", simfs_get_cwd());
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    char names[64][SIMFS_MAX_NAME];
    simfs_type_t types[64];
    int count = simfs_list_dir(NULL, names, types, 64);
    
    int dir_count = 0, file_count = 0;
    
    for (int i = 0; i < count; i++) {
        if (types[i] == SIMFS_TYPE_DIR) dir_count++;
        else file_count++;
    }
    
    int printed = 0;
    for (int i = 0; i < count; i++) {
        if (types[i] == SIMFS_TYPE_DIR) {
            vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
            printed++;
            if (printed == count) {
                printf("+-- %s/\n", names[i]);
            } else {
                printf("|-- %s/\n", names[i]);
            }
        }
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (int i = 0; i < count; i++) {
        if (types[i] == SIMFS_TYPE_FILE) {
            printed++;
            if (printed == count) {
                printf("+-- %s\n", names[i]);
            } else {
                printf("|-- %s\n", names[i]);
            }
        }
    }
    
    printf("\n");
    print_int(dir_count);
    printf(" directories, ");
    print_int(file_count);
    printf(" files\n");
}

static void cmd_echo(const char *args) {
    printf("%s\n", args);
}

static void cmd_calc(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: calc <num1> <op> <num2>\n");
        printf("Operators: + - * / %%\n");
        return;
    }
    
    int a = 0, b = 0;
    char op = 0;
    int i = 0;
    bool negative_a = false, negative_b = false;
    
    while (args[i] == ' ') i++;
    if (args[i] == '-') { negative_a = true; i++; }
    while (args[i] >= '0' && args[i] <= '9') {
        a = a * 10 + (args[i] - '0');
        i++;
    }
    if (negative_a) a = -a;
    
    while (args[i] == ' ') i++;
    if (args[i] == '+' || args[i] == '-' || args[i] == '*' || 
        args[i] == '/' || args[i] == '%') {
        op = args[i++];
    } else {
        printf("Error: invalid operator\n");
        return;
    }
    
    while (args[i] == ' ') i++;
    if (args[i] == '-') { negative_b = true; i++; }
    while (args[i] >= '0' && args[i] <= '9') {
        b = b * 10 + (args[i] - '0');
        i++;
    }
    if (negative_b) b = -b;
    
    int result = 0;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) { printf("Error: division by zero\n"); return; }
            result = a / b;
            break;
        case '%':
            if (b == 0) { printf("Error: modulo by zero\n"); return; }
            result = a % b;
            break;
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    print_int(a);
    printf(" %c ", op);
    print_int(b);
    printf(" = ");
    print_int(result);
    printf("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void cmd_history(void) {
    if (history_count == 0) {
        printf("Command history is empty.\n");
        return;
    }
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("\nCommand History:\n");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("------------------------------------\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    for (int i = 0; i < history_count; i++) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        printf(" ");
        if (i + 1 < 10) printf(" ");
        print_int(i + 1);
        printf("  ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        printf("%s\n", history[i]);
    }
    
    printf("\nTotal: ");
    print_int(history_count);
    printf(" command(s)\n");
}

static void cmd_reboot(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("Rebooting system...\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    for (volatile int i = 0; i < 10000000; i++);
    
    uint8_t temp;
    cli();
    do {
        temp = inb(0x64);
        if (temp & 1) inb(0x60);
    } while (temp & 2);
    outb(0x64, 0xFE);
    for(;;) hlt();
}

static void cmd_halt(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("System halted. Safe to power off.\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    cli();
    for(;;) hlt();
}

static void add_to_history(const char *cmd) {
    if (cmd[0] == '\0') return;
    
    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count++], cmd);
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[MAX_HISTORY - 1], cmd);
    }
}

static void execute_command(const char *cmd) {
    if (cmd[0] == '\0') return;
    
    add_to_history(cmd);
    
    char command[32] = {0};
    int i = 0, j = 0;
    
    while (cmd[i] == ' ') i++;
    while (cmd[i] && cmd[i] != ' ' && j < 31) {
        command[j++] = cmd[i++];
    }
    command[j] = '\0';
    
    while (cmd[i] == ' ') i++;
    const char *args = &cmd[i];
    
    if (strcmp(command, "help") == 0) cmd_help();
    else if (strcmp(command, "clear") == 0) { vga_clear(); show_welcome(); }
    else if (strcmp(command, "info") == 0) cmd_info();
    else if (strcmp(command, "uname") == 0) cmd_uname();
    else if (strcmp(command, "uptime") == 0) cmd_uptime();
    else if (strcmp(command, "date") == 0) cmd_date();
    else if (strcmp(command, "free") == 0) cmd_free();
    else if (strcmp(command, "tree") == 0) cmd_tree();
    else if (strcmp(command, "ls") == 0) cmd_ls();
    else if (strcmp(command, "pwd") == 0) cmd_pwd();
    else if (strcmp(command, "cd") == 0) cmd_cd(args);
    else if (strcmp(command, "mkdir") == 0) cmd_mkdir(args);
    else if (strcmp(command, "rmdir") == 0) cmd_rmdir(args);
    else if (strcmp(command, "touch") == 0) cmd_touch(args);
    else if (strcmp(command, "cat") == 0) cmd_cat(args);
    else if (strcmp(command, "write") == 0) cmd_write(args);
    else if (strcmp(command, "rm") == 0) cmd_rm(args);
    else if (strcmp(command, "echo") == 0) cmd_echo(args);
    else if (strcmp(command, "calc") == 0) cmd_calc(args);
    else if (strcmp(command, "history") == 0) cmd_history();
    else if (strcmp(command, "reboot") == 0) cmd_reboot();
    else if (strcmp(command, "halt") == 0) cmd_halt();
    else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        printf("%s: command not found\n", command);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        printf("Type 'help' for available commands\n");
    }
}

static void read_line(char *buffer, size_t max_len) {
    size_t pos = 0;
    
    while (1) {
        char c = keyboard_getchar();
        
        if (c == '\n') {
            buffer[pos] = '\0';
            printf("\n");
            return;
        }
        
        if (c == '\b') {
            if (pos > 0) {
                pos--;
                printf("\b");
            }
            continue;
        }
        
        if (c >= 32 && c <= 126 && pos < max_len - 1) {
            buffer[pos++] = c;
            printf("%c", c);
        }
    }
}

void shell_init(void) {
    simfs_init();
    show_welcome();
}

void shell_run(void) {
    while (1) {
        show_prompt();
        read_line(input_buffer, BUFFER_SIZE);
        execute_command(input_buffer);
    }
}
