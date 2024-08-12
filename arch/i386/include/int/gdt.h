#pragma once

#include <common.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} PACKED gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} PACKED gdt_ptr_t;

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0, ss0, esp1, ss1;
    uint32_t esp2, ss2, cr3;
    uint32_t eip, eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} PACKED tss_entry_t;


// gdt
void init_gdt();
void set_gdt_gate(uint8_t n, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);

// tss
extern void tss_flush();
void write_tss(uint8_t n, uint16_t ss0, uint32_t esp0);
void set_kernel_stack(uint32_t stack);