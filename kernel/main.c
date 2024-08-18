#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include <ssfn.h>
#include <stdio.h>
#include <multiboot.h>

#include <fs/skbdfs.h>

#include <hw/ata.h>
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

static void ssfn_cputs(char *s, uint32_t fg) {
    ssfn_dst.fg = fg;
    vesa_puts(s);
    ssfn_dst.fg = 0xFFFFFFFF;
}

void kernel_main() {
    vesa_puts("\nWelcome to ");
    ssfn_cputs("Radiant OS", 0xFF00FFFF);
    vesa_puts("!\n"); // just a minor detail cuz im a lil crazy for them.

    return;
}

extern struct FADT *fadt;

void kernel_init(struct mboot_info *mboot_ptr) {
    init_serial();
    serial_puts("Serial output successfully initialized.\n");

    init_gdt();
    init_idt();

    // outb(PIC_MASTER_DAT, 0xF9);
    // outb(PIC_SLAVE_DAT, 0xFF);

    // at f = 1193 Hz, T_irq = 0.999847 ms, the closest possible to 1 ms.
    pit_install(1193);

    init_paging();

    struct vbe_mode_info *vbe_info;
    vbe_info = (struct vbe_mode_info*) mboot_ptr->vbe_mode_info;
    init_vbe(vbe_info);

    ssfn_src = (ssfn_font_t *) &_binary_console_sfn_start;
    ssfn_dst.ptr = (uint8_t*) LFB_VADDR;
    ssfn_dst.p = vbe_info->pitch; // bytes per line
    ssfn_dst.fg = 0xFFFFFFFF; // white on black
    ssfn_dst.bg = 0x00000000;
    ssfn_dst.x = 1;
    ssfn_dst.y = 0;

    ssfn_putc('[');
    ssfn_cputs("+", 0xFF00FF00); // green
    vesa_puts("] SSFN has been configured.\n");

    // enable interrupts
    asm volatile("sti");

    init_acpi();
    ssfn_putc('[');
    ssfn_cputs("+", 0xFF00FF00);
    vesa_puts("] ACPI initialization completed.\n");

    uint32_t size = get_drive_size(0);

    init_fs();
    printf("Device detected: id: 0, size: %u MiB\n", size / 1024);

    kernel_main();
}
