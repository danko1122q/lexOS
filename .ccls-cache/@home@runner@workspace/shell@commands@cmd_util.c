/* ============================================
 * shell/commands/cmd_util.c - Utility Commands
 * ============================================ */
#include "cmd_registry.h"
#include "../../lib/stdio/stdio.h"
#include "../../drivers/rtc/rtc.h"

static int cmd_date(int argc, char **argv) {
    (void)argc;
    (void)argv;
    rtc_time_t time;
    rtc_get_time(&time);
    
    const char *months[] = {
        "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    printf("%s %d %d %02d:%02d:%02d\n",
           months[time.month], time.day, time.year,
           time.hour, time.minute, time.second);
    return 0;
}

static int cmd_calc(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: calc <num1> <op> <num2>\n");
        printf("Operators: + - * /\n");
        return -1;
    }
    
    // Simple string to int conversion
    int a = 0, b = 0;
    for (int i = 0; argv[1][i]; i++) {
        if (argv[1][i] >= '0' && argv[1][i] <= '9') {
            a = a * 10 + (argv[1][i] - '0');
        }
    }
    for (int i = 0; argv[3][i]; i++) {
        if (argv[3][i] >= '0' && argv[3][i] <= '9') {
            b = b * 10 + (argv[3][i] - '0');
        }
    }
    
    int result = 0;
    switch (argv[2][0]) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': 
            if (b == 0) {
                printf("Error: Division by zero\n");
                return -1;
            }
            result = a / b;
            break;
        default:
            printf("Error: Unknown operator\n");
            return -1;
    }
    
    printf("%d\n", result);
    return 0;
}

void register_util_commands(void) {
    register_command("date", cmd_date, "Display current date and time");
    register_command("calc", cmd_calc, "Simple calculator");
}
