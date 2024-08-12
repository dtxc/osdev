#include <int/gdt.h>
#include <string.h>

#define GDT_ENTRY_NUM 6

extern void gdt_flush(uint32_t);

gdt_entry_t gdt_entries[GDT_ENTRY_NUM];
gdt_ptr_t gdt_ptr;

tss_entry_t tss_entry;


void set_gdt_gate(uint8_t n, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt_entries[n].base_low     = (base & 0xFFFF);
    gdt_entries[n].base_mid     = (base >> 16) & 0xFF;
    gdt_entries[n].base_high    = (base >> 24) & 0xFF;

    gdt_entries[n].limit_low    = (limit & 0xFFFF);
    gdt_entries[n].granularity  = (limit >> 16) & 0x0F;
    gdt_entries[n].granularity  |= granularity & 0xF0;
    gdt_entries[n].access       = access;
}

void write_tss(uint8_t n, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry_t);

    set_gdt_gate(n, base, limit, 0xE9, 0x00);
    memset(&tss_entry, 0, sizeof(tss_entry_t));

    tss_entry.ss0   = ss0;
    tss_entry.esp0  = esp0;

    tss_entry.cs = 0x0B;
    tss_entry.ss = tss_entry.ds = tss_entry.es = 0x13;
    tss_entry.fs = tss_entry.gs = 0x13;
}

void set_kernel_stack(uint32_t stack) {
    tss_entry.esp0 = stack;
}

void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRY_NUM) - 1;
    gdt_ptr.base = (uint32_t) &gdt_entries;

    set_gdt_gate(0, 0, 0, 0,0);                 // null segment
    set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code segment
    set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data segment
    set_gdt_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // usermode code segment
    set_gdt_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // usermode data segment
    write_tss(5, 0x10, 0x0);                    // task state segment

    gdt_flush((uint32_t) &gdt_ptr);
    tss_flush();
}