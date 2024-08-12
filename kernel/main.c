#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include <ssfn.h>

#include <stdio.h>
#include <multiboot.h>

#include <asm/io.h>
#include <int/idt.h>
#include <int/gdt.h>
#include <hal/acpi.h>
#include <mm/kheap.h>
#include <mm/paging.h>
#include <int/timer.h>
#include <video/vga.h>
#include <video/vbe.h>

extern uint8_t _binary_console_sfn_start;

static void ssfn_puts(char *s) {
    while (*s != '\0') {
        if (*s == '\n') {
            ssfn_dst.y++;
            ssfn_dst.x = 1;
        }

        ssfn_putc(*s);
        s++;
    }
}

void kernel_main() {
    ssfn_puts("hello world");
    
    return;
}

void kernel_init(struct mboot_info *mboot_ptr) {
    clear();

    init_serial();
    serial_puts("Serial output successfully initialized.\n");

    init_gdt();
    init_idt();

    // outb(PIC_MASTER_DAT, 0xF9);
    // outb(PIC_SLAVE_DAT, 0xFF);

    pit_install(1193);

    init_paging();

    struct vbe_mode_info *vbe_info;
    vbe_info = (struct vbe_mode_info*) mboot_ptr->vbe_mode_info;
    init_vbe(vbe_info);

    ssfn_src = &_binary_console_sfn_start;
    ssfn_dst.ptr = (uint8_t*) LFB_VADDR;
    ssfn_dst.p = vbe_info->pitch;
    ssfn_dst.fg = 0xFFFFFFFF;
    ssfn_dst.bg = 0x00000000;
    ssfn_dst.x = 1;
    ssfn_dst.y = 0;

    asm volatile("sti");

    init_acpi();
    // ssfn_puts("ACPI initialization completed.\n");

    kernel_main(vbe_info);
}
