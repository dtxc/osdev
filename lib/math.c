#include <stdint.h>

// __udivdi3 is not available on 32 bit
uint64_t __udivdi3(uint64_t num, uint64_t den) {
    uint64_t quot = 0, qbit = 1;

    if (den == 0) {
        // TODO: int 0x00 handler
        asm volatile("int $0x00");

        __builtin_unreachable();
    }

    while ((int64_t)den >= 0 && den < num) {
        den <<= 1;
        qbit <<= 1;
    }

    while (qbit > 0) {
        if (num >= den) {
            num -= den;
            quot |= qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    return quot;
}
