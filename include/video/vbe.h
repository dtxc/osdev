#pragma once

#include <common.h>

#define LFB_PHYS_ADDR   0xFD000000
#define LFB_VADDR       0xD0000000 
#define LFB_SIZE        0x00200000 // 2 mib, enough for 600x800x32

struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch; // bytes per scanline

    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp; // bits per pixel
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size; // in KB
    uint8_t reserved1[206];
} __attribute__((packed));

void init_vbe(struct vbe_mode_info *vbe_info);
void putpixel(int x, int y, uint32_t color);