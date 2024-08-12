#include <asm/io.h>
#include <video/vbe.h>

uint32_t *lfb;
uint32_t width, height, bpp, pitch;

void putpixel(int x, int y, uint32_t color) {
    *(lfb + y * (pitch / 4) + x) = color;
}

void init_vbe(struct vbe_mode_info *vbe_info) {
    lfb = (uint32_t *) LFB_VADDR;

    serial_printf(
        "framebuffer initialized with VA: 0x%x, PA: 0x%x\n",
        LFB_VADDR, LFB_PHYS_ADDR
    );

    width = vbe_info->width;
    height = vbe_info->height;
    bpp = vbe_info->bpp;
    pitch = vbe_info->pitch;
}