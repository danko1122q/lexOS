#include "irq.h"
#include "kernel.h"

static irq_handler_t irq_handlers[16] = {0};

static void pic_remap(void) {
    // ICW1
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // ICW2 - remap
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    
    // ICW3
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    
    // ICW4
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Clear masks - enable all
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void irq_init(void) {
    pic_remap();
    __asm__ volatile("sti");  // CRITICAL: Enable interrupts!
}

void irq_install_handler(int irq, irq_handler_t handler) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_handler(registers_t *regs) {
    if (regs->int_no >= 32 && regs->int_no <= 47) {
        int irq = regs->int_no - 32;
        if (irq_handlers[irq]) {
            irq_handlers[irq](regs);
        }
    }
    
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

void isr_handler(registers_t *regs) {
    (void)regs;
}
