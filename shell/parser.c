/* ============================================
 * shell/parser.c - Command Parser
 * ============================================ */
#include "parser.h"
#include "../lib/string/string.h"

void parse_command(const char *input, parsed_command_t *cmd) {
    static char buffer[256];
    strcpy(buffer, input);
    
    cmd->argc = 0;
    char *token = buffer;
    bool in_quote = false;
    
    for (int i = 0; buffer[i]; i++) {
        if (buffer[i] == '"') {
            in_quote = !in_quote;
            buffer[i] = '\0';
        } else if (buffer[i] == ' ' && !in_quote) {
            buffer[i] = '\0';
            if (strlen(token) > 0 && cmd->argc < MAX_ARGS) {
                cmd->argv[cmd->argc++] = token;
            }
            token = &buffer[i + 1];
        }
    }
    
    if (strlen(token) > 0 && cmd->argc < MAX_ARGS) {
        cmd->argv[cmd->argc++] = token;
    }
}
