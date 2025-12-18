#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../include/types.h"

void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_has_input(void);
void keyboard_readline(char *buffer, size_t max_len);

#endif
