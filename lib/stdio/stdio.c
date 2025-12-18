#include "stdio.h"
#include "../string/string.h"
#include "../../drivers/vga/vga.h"

void putchar(char c) {
    vga_putchar(c);
}

static void print_int(char *buf, int *pos, int value, int base) {
    char temp[32];
    int i = 0;
    bool negative = false;
    
    if (value < 0 && base == 10) {
        negative = true;
        value = -value;
    }
    
    if (value == 0) {
        temp[i++] = '0';
    } else {
        while (value > 0) {
            int digit = value % base;
            temp[i++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
            value /= base;
        }
    }
    
    if (negative) temp[i++] = '-';
    
    while (i > 0) {
        buf[(*pos)++] = temp[--i];
    }
}

int vsprintf(char *str, const char *format, va_list args) {
    int pos = 0;
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd':
                case 'i':
                    print_int(str, &pos, va_arg(args, int), 10);
                    break;
                case 'x':
                    print_int(str, &pos, va_arg(args, unsigned int), 16);
                    break;
                case 'c':
                    str[pos++] = (char)va_arg(args, int);
                    break;
                case 's': {
                    char *s = va_arg(args, char*);
                    while (*s) str[pos++] = *s++;
                    break;
                }
                case '%':
                    str[pos++] = '%';
                    break;
            }
        } else {
            str[pos++] = *format;
        }
        format++;
    }
    
    str[pos] = '\0';
    return pos;
}

int sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(str, format, args);
    va_end(args);
    return ret;
}

int printf(const char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    int ret = vsprintf(buffer, format, args);
    va_end(args);
    vga_puts(buffer);
    return ret;
}
