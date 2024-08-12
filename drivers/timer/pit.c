#include <asm/io.h>
#include <int/isr.h>
#include <int/timer.h>

static volatile uint32_t tick = 0;

static void pit_callback(regs_t *regs) {
    /*
        for freq = 1193 Hz, 1 tick = 0.999847 ms.
        error/day = -13.22 s
    */
    tick++;
    irq_ack(0);
}

void pit_sleep(uint32_t ms) {
    uint32_t start = tick;

    while (pit_get_ticks() < start + ms) {
        asm("hlt");
    }
}

void pit_install(uint32_t freq) {
    register_interrupt_handler(IRQ(0), pit_callback);

    uint32_t div = 1193182 / freq;
    outb(PIT_CMD, PIT_SET);

    uint8_t low = div & 0xFF;
    uint8_t high = (div >> 8) & 0xFF;

    outb(PIT_DAT0, low);
    outb(PIT_DAT0, high);
}

uint32_t pit_get_ticks() {
    return tick;
}