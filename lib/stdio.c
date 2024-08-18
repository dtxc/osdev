#include <stdio.h>
#include <string.h>
#include <video/vbe.h>
#include <video/vga.h>

static void vfprintf(file_t *fp, const char *s, va_list list) {
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
                if (fp == stdout) {
                    #ifndef VESA_MODE
                    puts(ptr);
                    #else
                    vesa_puts(ptr);
                    #endif
                } else {
                    fwrite(fp, 10, (uint8_t *) ptr);
                }
            } else if (next == 'u') {
                uint32_t arg = va_arg(list, uint32_t);
                char ptr[9];
                itoa(ptr, arg);
                if (fp == stdout) {
                    #ifndef VESA_MODE
                    puts(ptr);
                    #else
                    vesa_puts(ptr);
                    #endif
                } else {
                    fwrite(fp, 9, (uint8_t *) ptr);
                }
            } else if (next == 'x') {
                uint32_t arg = va_arg(list, uint32_t);
                char ptr[9];
                int2hex(ptr, arg);
                if (fp == stdout) {
                    #ifndef VESA_MODE
                    puts(ptr);
                    #else
                    vesa_puts(ptr);
                    #endif
                } else {
                    fwrite(fp, 9, (uint8_t *) ptr);
                }
            } else if (next == 'c') {
                char arg = va_arg(list, int);
                if (fp == stdout) {
                    #ifndef VESA_MODE
                    putc(ptr);
                    #else
                    vesa_putc(arg);
                    #endif
                } else {
                    fwrite(fp, 1, (uint8_t *) &arg);
                }
            } else if (next == 's') {
                char *ptr = va_arg(list, char*);
                if (fp == stdout) {
                    #ifndef VESA_MODE
                    puts(ptr);
                    #else
                    vesa_puts(ptr);
                    #endif
                } else {
                    fwrite(fp, strlen(ptr), (uint8_t *) ptr);
                }
            } else if (next == '%') {
                if (fp == stdout) {
                    #ifndef VESA_MODE
                    putc('%');
                    #else
                    vesa_putc('%');
                    #endif
                } else {
                    fwrite(fp, 1, (uint8_t *) "%");
                }
            }
        } else {
            if (fp == stdout) {
                #ifndef VESA_MODE
                putc(c);
                #else
                vesa_putc(c);
                #endif
            } else {
                fwrite(fp, 1, (uint8_t *) &c);
            }
        }
        i++;
    }
}

void fprintf(file_t *fp, const char *s, ...) {
    va_list arg;
    va_start(arg, s);
    vfprintf(fp, s, arg);
    va_end(arg);
}

void printf(const char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stdout, fmt, arg);
    va_end(arg);
}
