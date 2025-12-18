#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

#define KERNEL_VERSION_MAJOR 2
#define KERNEL_VERSION_MINOR 0
#define KERNEL_VERSION_PATCH 0
#define KERNEL_VERSION_STRING "2.0.0"

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void cli(void) {
    __asm__ volatile("cli");
}

static inline void sti(void) {
    __asm__ volatile("sti");
}

static inline void hlt(void) {
    __asm__ volatile("hlt");
}

void kernel_panic(const char *message);

#endif
