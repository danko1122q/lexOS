/* ================================================
 * shell/shell.c - FIXED VERSION v1
 * Fixed: printf %d format specifier issues
 * Added: Manual integer to string conversion
 * ================================================ */
#include "shell.h"
#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "../lib/stdio/stdio.h"
#include "../lib/string/string.h"
#include "../kernel/timer.h"
#include "../kernel/memory.h"
#include "../kernel/kernel.h"

#define BUFFER_SIZE 256
#define MAX_HISTORY 10
#define MAX_FILES 20
#define MAX_DIRS 10
#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80

static char input_buffer[BUFFER_SIZE];
static char history[MAX_HISTORY][BUFFER_SIZE];
static int history_count = 0;

// Enhanced filesystem simulation
static char current_dir[BUFFER_SIZE] = "/";
static char files[MAX_FILES][64] = {0};
static char file_contents[MAX_FILES][1024] = {0};
static int file_count = 0;
static char directories[MAX_DIRS][64] = {0};
static int dir_count = 0;

// Forward declarations
static char keyboard_scancode_to_char(uint8_t scancode);

// Helper function to convert integer to string
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
    
    // Reverse the string
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

// Helper to print integer
static void print_int(int num) {
    char buffer[20];
    itoa_custom(num, buffer);
    printf("%s", buffer);
}

// Helper to print integer with padding (for time format)
static void print_int_padded(int num) {
    if (num < 10) {
        printf("0");
    }
    print_int(num);
}

static void show_welcome(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    printf("\n╔════════════════════════════════════╗\n");
    printf("  ║   Welcome to LexOS Shell v0.0.1   ║\n");
    printf("  ╚════════════════════════════════════╝\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("  Type 'help' for available commands\n\n");
}

static void show_prompt(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    printf("lexos");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf(":");
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    printf("%s", current_dir);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("$ ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static char wait_for_key(void) {
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            
            if (scancode == 0x48) return 'u';      // Arrow Up
            if (scancode == 0x50) return 'd';      // Arrow Down
            if (scancode == 0x49) return 'U';      // Page Up
            if (scancode == 0x51) return 'D';      // Page Down
            if (scancode == 0x01) return 'q';      // ESC
            if (scancode == 0x1C) return '\n';     // Enter
            if (scancode == 0x39) return ' ';      // Space
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
        "  info      - Display detailed system information",
        "  uname     - Print system information",
        "  uptime    - Show system uptime",
        "  date      - Display current uptime",
        "  free      - Display memory usage",
        "  clear     - Clear the screen",
        "",
        "File & Directory:",
        "  ls        - List files and directories",
        "  pwd       - Print working directory",
        "  cd        - Change directory",
        "  mkdir     - Create a new directory",
        "  touch     - Create a new file",
        "  cat       - Display file contents",
        "  rm        - Remove a file",
        "  tree      - Display directory tree structure",
        "",
        "Text Editor:",
        "  write     - Simple text editor (write <filename>)",
        "",
        "Utilities:",
        "  echo      - Print text to screen",
        "  calc      - Simple calculator",
        "  history   - Show command history",
        "  test      - Keyboard test mode",
        "",
        "System Control:",
        "  reboot    - Reboot the system",
        "  halt      - Halt the system",
        "",
        "Examples:",
        "  calc 5 + 3",
        "  echo Hello World",
        "  touch myfile.txt",
        "  write myfile.txt",
        "  mkdir mydir",
        "  tree",
        "",
        "Navigation:",
        "  Arrow Up/Down  - Scroll help",
        "  Page Up/Down   - Fast scroll",
        "  ESC            - Exit help",
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
            } else if (line[0] == '=' || line[2] == '=') {
                vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                printf("%s\n", line);
            } else if (line[strlen(line)-1] == ':') {
                vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
                printf("%s\n", line);
            } else if (line[0] == ' ' && line[1] == ' ') {
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                printf("%s\n", line);
            } else {
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                printf("%s\n", line);
            }
        }
        
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("\n[↑↓/PgUp/PgDn: Scroll | ESC/Q: Exit] ");
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
        } else if (key == 'u') {
            if (scroll_pos > 0) scroll_pos--;
        } else if (key == 'd' || key == ' ' || key == '\n') {
            if (scroll_pos + max_lines < total_lines) scroll_pos++;
        } else if (key == 'U') {
            scroll_pos -= 5;
            if (scroll_pos < 0) scroll_pos = 0;
        } else if (key == 'D') {
            scroll_pos += 5;
            if (scroll_pos + max_lines > total_lines) {
                scroll_pos = total_lines - max_lines;
                if (scroll_pos < 0) scroll_pos = 0;
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
    
    uint32_t total, used, free;
    memory_stats(&total, &used, &free);
    
    if (total > 0) {
        printf("Memory:       ");
        print_int(total/1024);
        printf(" KB total, ");
        print_int(free/1024);
        printf(" KB free\n");
    } else {
        printf("Memory:       Not available\n");
    }
    
    uint32_t sec = timer_get_seconds();
    uint32_t hours = sec / 3600;
    uint32_t minutes = (sec % 3600) / 60;
    uint32_t seconds = sec % 60;
    
    printf("Uptime:       ");
    print_int(hours);
    printf("h ");
    print_int(minutes);
    printf("m ");
    print_int(seconds);
    printf("s\n");
    
    printf("Shell:        LexOS Shell v0.0.1\n");
    printf("Author:       Davanico (danko1122)\n\n");
}

static void cmd_uname(void) {
    printf("LexOS v%s x86\n", KERNEL_VERSION_STRING);
}

static void cmd_uptime(void) {
    uint32_t sec = timer_get_seconds();
    uint32_t hours = sec / 3600;
    uint32_t minutes = (sec % 3600) / 60;
    uint32_t seconds = sec % 60;
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    printf("System uptime: ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    print_int(hours);
    printf(" hours, ");
    print_int(minutes);
    printf(" minutes, ");
    print_int(seconds);
    printf(" seconds\n");
}

static void cmd_date(void) {
    uint32_t sec = timer_get_seconds();
    uint32_t hours = sec / 3600;
    uint32_t minutes = (sec % 3600) / 60;
    uint32_t seconds = sec % 60;
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("System Uptime (RTC not implemented):\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("  Time: ");
    print_int_padded(hours);
    printf(":");
    print_int_padded(minutes);
    printf(":");
    print_int_padded(seconds);
    printf(" (HH:MM:SS)\n");
    printf("  Total seconds: ");
    print_int(sec);
    printf("\n");
}

static void cmd_free(void) {
    uint32_t total = 0, used = 0, free_mem = 0;
    memory_stats(&total, &used, &free_mem);
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("\nMemory Usage:\n");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("─────────────────────────────────\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    if (total > 0) {
        uint32_t total_kb = total / 1024;
        uint32_t used_kb = used / 1024;
        uint32_t free_kb = free_mem / 1024;
        
        printf("         Total    Used    Free\n");
        printf("Mem:  ");
        
        // Print total (right-aligned to 7 chars)
        char buf[20];
        itoa_custom(total_kb, buf);
        int len = strlen(buf);
        for (int i = 0; i < (7 - len); i++) printf(" ");
        printf("%s ", buf);
        
        // Print used
        itoa_custom(used_kb, buf);
        len = strlen(buf);
        for (int i = 0; i < (7 - len); i++) printf(" ");
        printf("%s ", buf);
        
        // Print free
        itoa_custom(free_kb, buf);
        len = strlen(buf);
        for (int i = 0; i < (7 - len); i++) printf(" ");
        printf("%s KB\n", buf);
        
        uint32_t used_percent = (used * 100) / total;
        printf("\nUsage: ");
        print_int(used_percent);
        printf("%%\n");
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        printf("Error: Memory statistics not available\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

static void cmd_tree(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("%s\n", current_dir);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    int total_items = 0;
    
    for (int i = 0; i < dir_count; i++) {
        vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
        if (i == dir_count - 1 && file_count == 0) {
            printf("└── %s/\n", directories[i]);
        } else {
            printf("├── %s/\n", directories[i]);
        }
        total_items++;
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (int i = 0; i < file_count; i++) {
        if (i == file_count - 1) {
            printf("└── %s\n", files[i]);
        } else {
            printf("├── %s\n", files[i]);
        }
        total_items++;
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("\n");
    print_int(dir_count);
    printf(" directories, ");
    print_int(file_count);
    printf(" files\n");
}

static void cmd_ls(void) {
    int total_items = dir_count + file_count;
    
    if (total_items == 0) {
        printf("(empty)\n");
        return;
    }
    
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    for (int i = 0; i < dir_count; i++) {
        printf("%s/  ", directories[i]);
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (int i = 0; i < file_count; i++) {
        printf("%s  ", files[i]);
    }
    printf("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void cmd_pwd(void) {
    printf("%s\n", current_dir);
}

static void cmd_cd(const char *args) {
    if (args[0] == '\0' || strcmp(args, "/") == 0) {
        strcpy(current_dir, "/");
        printf("Changed to: %s\n", current_dir);
        return;
    }
    
    if (strcmp(args, "..") == 0) {
        strcpy(current_dir, "/");
        printf("Changed to: %s\n", current_dir);
        return;
    }
    
    for (int i = 0; i < dir_count; i++) {
        if (strcmp(directories[i], args) == 0) {
            strcpy(current_dir, "/");
            strcat(current_dir, args);
            printf("Changed to: %s\n", current_dir);
            return;
        }
    }
    
    printf("cd: %s: No such directory\n", args);
}

static void cmd_mkdir(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: mkdir <dirname>\n");
        return;
    }
    
    char dirname[64] = {0};
    int j = 0;
    for (int i = 0; args[i] && args[i] != ' ' && j < 63; i++) {
        dirname[j++] = args[i];
    }
    dirname[j] = '\0';
    
    if (dirname[0] == '\0') {
        printf("Usage: mkdir <dirname>\n");
        return;
    }
    
    if (dir_count >= MAX_DIRS) {
        printf("mkdir: error: maximum directories reached\n");
        return;
    }
    
    for (int i = 0; i < dir_count; i++) {
        if (strcmp(directories[i], dirname) == 0) {
            printf("mkdir: cannot create directory '%s': File exists\n", dirname);
            return;
        }
    }
    
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i], dirname) == 0) {
            printf("mkdir: cannot create directory '%s': File exists\n", dirname);
            return;
        }
    }
    
    strcpy(directories[dir_count], dirname);
    dir_count++;
    
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("mkdir: create '%s'\n", dirname);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void cmd_touch(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: touch <filename>\n");
        return;
    }
    
    char filename[64] = {0};
    int j = 0;
    for (int i = 0; args[i] && args[i] != ' ' && j < 63; i++) {
        filename[j++] = args[i];
    }
    filename[j] = '\0';
    
    if (filename[0] == '\0') {
        printf("Usage: touch <filename>\n");
        return;
    }
    
    if (file_count >= MAX_FILES) {
        printf("touch: error: maximum files reached\n");
        return;
    }
    
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i], filename) == 0) {
            printf("touch: '%s' already exists\n", filename);
            return;
        }
    }
    
    for (int i = 0; i < dir_count; i++) {
        if (strcmp(directories[i], filename) == 0) {
            printf("touch: cannot create file '%s': Is a directory\n", filename);
            return;
        }
    }
    
    strcpy(files[file_count], filename);
    file_contents[file_count][0] = '\0';
    file_count++;
    
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("touch: created file '%s'\n", filename);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void cmd_cat(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: cat <filename>\n");
        return;
    }
    
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i], args) == 0) {
            if (file_contents[i][0] == '\0') {
                printf("(empty file)\n");
            } else {
                printf("%s\n", file_contents[i]);
            }
            return;
        }
    }
    
    printf("cat: %s: No such file or directory\n", args);
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
    
    int file_idx = -1;
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i], filename) == 0) {
            file_idx = i;
            break;
        }
    }
    
    if (file_idx == -1) {
        if (file_count >= MAX_FILES) {
            printf("write: error: maximum files reached\n");
            return;
        }
        strcpy(files[file_count], filename);
        file_idx = file_count;
        file_count++;
    }
    
    vga_clear();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("=== LexOS Text Editor ===\n");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("Editing: %s\n", filename);
    printf("Press ESC to save and exit\n");
    printf("Press F1 to exit without saving\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("─────────────────────────────────────\n");
    
    char buffer[1024];
    strcpy(buffer, file_contents[file_idx]);
    int pos = strlen(buffer);
    printf("%s", buffer);
    
    bool save = true;
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t sc = inb(0x60);
            
            if (sc == 0x01) {
                save = true;
                break;
            }
            
            if (sc == 0x3B) {
                save = false;
                break;
            }
            
            if (sc == 0x0E) {
                if (pos > 0) {
                    pos--;
                    buffer[pos] = '\0';
                    printf("\b");
                }
                continue;
            }
            
            if (sc == 0x1C) {
                if (pos < 1023) {
                    buffer[pos++] = '\n';
                    buffer[pos] = '\0';
                    printf("\n");
                }
                continue;
            }
            
            char c = keyboard_scancode_to_char(sc);
            if (c >= 32 && c <= 126 && pos < 1023) {
                buffer[pos++] = c;
                buffer[pos] = '\0';
                printf("%c", c);
            }
        }
    }
    
    if (save) {
        strcpy(file_contents[file_idx], buffer);
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

static void cmd_rm(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: rm <filename>\n");
        return;
    }
    
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i], args) == 0) {
            for (int j = i; j < file_count - 1; j++) {
                strcpy(files[j], files[j + 1]);
                strcpy(file_contents[j], file_contents[j + 1]);
            }
            file_count--;
            vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
            printf("rm: removed '%s'\n", args);
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            return;
        }
    }
    
    printf("rm: cannot remove '%s': No such file or directory\n", args);
}

static void cmd_echo(const char *args) {
    printf("%s\n", args);
}

static void cmd_calc(const char *args) {
    if (args[0] == '\0') {
        printf("Usage: calc <num1> <op> <num2>\n");
        printf("Operators: + - * / %%\n");
        printf("Example: calc 5 + 3\n");
        return;
    }
    
    int a = 0, b = 0;
    char op = 0;
    int i = 0;
    bool negative_a = false, negative_b = false;
    
    while (args[i] == ' ') i++;
    
    if (args[i] == '-') {
        negative_a = true;
        i++;
    }
    
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
        printf("Error: invalid operator (use: + - * / %%)\n");
        return;
    }
    
    while (args[i] == ' ') i++;
    
    if (args[i] == '-') {
        negative_b = true;
        i++;
    }
    
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
            if (b == 0) {
                printf("Error: division by zero\n");
                return;
            }
            result = a / b; 
            break;
        case '%':
            if (b == 0) {
                printf("Error: modulo by zero\n");
                return;
            }
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
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        printf("Command history is empty.\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        return;
    }
    
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("\nCommand History:\n");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("────────────────────────────────────\n");
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
    
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("────────────────────────────────────\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("Total: ");
    print_int(history_count);
    printf(" command(s)\n");
}

static void cmd_test(void) {
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("\n=== Keyboard Test Mode ===\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("Press any keys to see scancodes\n");
    printf("Press 'q' to quit test mode\n\n");
    
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t sc = inb(0x60);
            if (sc == 0x10) {
                printf("\n");
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                printf("Test mode exited.\n");
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                return;
            }
            if (!(sc & 0x80)) {
                printf("0x");
                char hex[3];
                hex[0] = "0123456789abcdef"[(sc >> 4) & 0xF];
                hex[1] = "0123456789abcdef"[sc & 0xF];
                hex[2] = '\0';
                printf("%s ", hex);
            }
        }
    }
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
    
    if (strcmp(command, "help") == 0) {
        cmd_help();
    } else if (strcmp(command, "clear") == 0) {
        vga_clear();
        show_welcome();
    } else if (strcmp(command, "info") == 0) {
        cmd_info();
    } else if (strcmp(command, "uname") == 0) {
        cmd_uname();
    } else if (strcmp(command, "uptime") == 0) {
        cmd_uptime();
    } else if (strcmp(command, "date") == 0) {
        cmd_date();
    } else if (strcmp(command, "free") == 0) {
        cmd_free();
    } else if (strcmp(command, "tree") == 0) {
        cmd_tree();
    } else if (strcmp(command, "ls") == 0) {
        cmd_ls();
    } else if (strcmp(command, "pwd") == 0) {
        cmd_pwd();
    } else if (strcmp(command, "cd") == 0) {
        cmd_cd(args);
    } else if (strcmp(command, "mkdir") == 0) {
        cmd_mkdir(args);
    } else if (strcmp(command, "touch") == 0) {
        cmd_touch(args);
    } else if (strcmp(command, "cat") == 0) {
        cmd_cat(args);
    } else if (strcmp(command, "write") == 0) {
        cmd_write(args);
    } else if (strcmp(command, "rm") == 0) {
        cmd_rm(args);
    } else if (strcmp(command, "echo") == 0) {
        cmd_echo(args);
    } else if (strcmp(command, "calc") == 0) {
        cmd_calc(args);
    } else if (strcmp(command, "history") == 0) {
        cmd_history();
    } else if (strcmp(command, "test") == 0) {
        cmd_test();
    } else if (strcmp(command, "reboot") == 0) {
        cmd_reboot();
    } else if (strcmp(command, "halt") == 0) {
        cmd_halt();
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        printf("%s: command not found\n", command);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        printf("Type 'help' to see available commands\n");
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
    show_welcome();
}

void shell_run(void) {
    while (1) {
        show_prompt();
        read_line(input_buffer, BUFFER_SIZE);
        execute_command(input_buffer);
    }
}
