#include <stdarg.h>
#include <string.h>
#include <asm/io.h>

int init_serial() {
    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03); // set divisor low to 3
    outb(COM1 + 1, 0x00); // divisor high
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    outb(COM1 + 4, 0x1E);
    outb(COM1 + 0, 0xAE);

    if (inb(COM1) != 0xAE) {
        return 1;
    }

    outb(COM1 + 4, 0x0F);
    return 0;
}

int is_transmit_empty() {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    while (is_transmit_empty() == 0);
    outb(COM1, c);
}

void serial_puts(char *s) {
    while (*s != '\0') {
        serial_putc(*s);
        s++;
    }
}

void serial_printf(const char *s, ...) {
    va_list list;
    va_start(list, s);

    int i = 0;
    while (1) {
        char c = s[i];
        if (c == 0) {
            break;
        }

        if (c == '%') {
            i++;
            char next = s[i];

            if (c == 0) {
                break;
            }

            if (next == 'd') {
                int arg = va_arg(list, int);
                char ptr[10];
                itoa(ptr, arg);
                serial_puts(ptr);
            } else if (next == 'u') {
                uint32_t arg = va_arg(list, uint32_t);
                char ptr[9];
                itoa(ptr, arg);
                serial_puts(ptr);
            } else if (next == 'x') {
                uint32_t arg = va_arg(list, uint32_t);
                char ptr[10];
                int2hex(ptr, arg);
                serial_puts(ptr);
            } else if (next == 'c') {
                char arg = va_arg(list, int);
                serial_putc(arg);
            } else if (next == 's') {
                char *arg = va_arg(list, char*);
                serial_puts(arg);
            } else if (next == '%') {
                serial_putc('%');
            }
        } else {
            serial_putc(c);
        }
        i++;
    }

    va_end(list);
}