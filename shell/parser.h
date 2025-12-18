/* ============================================
 * shell/parser.h - Command Parser Header
 * ============================================ */
#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 16

typedef struct {
    int argc;
    char *argv[MAX_ARGS];
} parsed_command_t;

void parse_command(const char *input, parsed_command_t *cmd);

#endif
