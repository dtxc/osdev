#include <stdio.h>
#include <string.h>

#include <video/vga.h>

void vprintf(const char *s, va_list list) {
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
                puts(ptr);
            } else if (next == 'u') {
                uint32_t arg = va_arg(list, uint32_t);
                char ptr[9];
                itoa(ptr, arg);
                puts(ptr);
            } else if (next == 'x') {
                uint32_t arg = va_arg(list, uint32_t);
                char ptr[9];
                int2hex(ptr, arg);
                puts(ptr);
            } else if (next == 'c') {
                char arg = va_arg(list, int);
                putc(arg);
            } else if (next == 's') {
                char *arg = va_arg(list, char*);
                puts(arg);
            } else if (next == '%') {
                putc('%');
            }
        } else {
            putc(c);
        }
        i++;
    }
}

void printf(const char *s, ...) {
    va_list arg;
    va_start(arg, s);
    vprintf(s, arg);
    va_end(arg);
}