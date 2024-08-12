#include <asm/io.h>
#include <int/isr.h>

#define INTERRUPT_NUM 256
isr_t interrupt_handlers[INTERRUPT_NUM];

void isr_handler(regs_t regs) {
    if (interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(&regs);
    } else {
        // unhandled interrupt
    }
}

void irq_ack(uint8_t int_no) {
    if (int_no >= 12) {
        outb(PIC_SLAVE_CMD, PIC_EIO);
    }
    outb(PIC_MASTER_CMD, PIC_EIO);
}

void irq_handler(regs_t regs) {
    if (interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(&regs);
    }
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}