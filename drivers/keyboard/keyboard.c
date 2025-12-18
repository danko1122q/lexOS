/* ================================================
 * drivers/keyboard/keyboard.c - POLLING MODE
 * No interrupts needed - direct polling
 * ================================================ */
#include "keyboard.h"
#include "../../include/kernel.h"

static const char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static const char scancode_to_ascii_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

static bool shift_pressed = false;

void keyboard_init(void) {
    // Clear keyboard buffer
    while (inb(0x64) & 1) {
        inb(0x60);
    }
}

bool keyboard_has_input(void) {
    return (inb(0x64) & 1) != 0;
}

char keyboard_getchar(void) {
    uint8_t scancode;
    
    while (1) {
        // Wait for data
        while (!(inb(0x64) & 1)) {
            // Small delay
            for (volatile int i = 0; i < 100; i++);
        }
        
        scancode = inb(0x60);
        
        // Handle key release
        if (scancode & 0x80) {
            scancode &= 0x7F;
            if (scancode == 0x2A || scancode == 0x36) {
                shift_pressed = false;
            }
            continue;
        }
        
        // Handle shift press
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = true;
            continue;
        }
        
        // Convert to ASCII
        if (scancode < 128) {
            char c;
            if (shift_pressed) {
                c = scancode_to_ascii_shift[scancode];
            } else {
                c = scancode_to_ascii[scancode];
            }
            
            if (c) {
                return c;
            }
        }
    }
}

void keyboard_readline(char *buffer, size_t max_len) {
    size_t pos = 0;
    
    while (pos < max_len - 1) {
        char c = keyboard_getchar();
        
        if (c == '\n') {
            buffer[pos] = '\0';
            return;
        }
        
        if (c == '\b') {
            if (pos > 0) {
                pos--;
            }
            continue;
        }
        
        if (c >= 32 && c <= 126) {
            buffer[pos++] = c;
        }
    }
    
    buffer[max_len - 1] = '\0';
}
