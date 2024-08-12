#pragma once

#include <common.h>

#define IRQ_BASE 32
#define IRQ(n) (IRQ_BASE + n)

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} regs_t;

typedef void (*isr_t) (regs_t*);

void register_interrupt_handler(uint8_t n, isr_t handler);
void irq_ack(uint8_t int_no);