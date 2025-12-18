#ifndef STDIO_H
#define STDIO_H

#include "../../include/types.h"

typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
void putchar(char c);

#endif
