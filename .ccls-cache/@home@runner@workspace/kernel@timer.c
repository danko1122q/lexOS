/* ================================================
 * kernel/timer.c
 * ================================================ */
#include "timer.h"
#include "irq.h"
#include "kernel.h"

static volatile uint32_t timer_ticks = 0;
static uint32_t timer_frequency = 0;

static void timer_handler(registers_t *regs) {
    (void)regs;
    timer_ticks++;
}

void timer_init(uint32_t frequency) {
    timer_frequency = frequency;
    irq_install_handler(0, timer_handler);
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

uint32_t timer_get_seconds(void) {
    return timer_ticks / timer_frequency;
}

void timer_sleep(uint32_t ms) {
    uint32_t target = timer_ticks + (ms * timer_frequency / 1000);
    while (timer_ticks < target) {
        __asm__ volatile("hlt");
    }
}
