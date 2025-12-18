#ifndef IRQ_H
#define IRQ_H

#include "../include/types.h"

#define IRQ0  32
#define IRQ1  33

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

typedef void (*irq_handler_t)(registers_t *);

void irq_init(void);
void irq_install_handler(int irq, irq_handler_t handler);

#endif
