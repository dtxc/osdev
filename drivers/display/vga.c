#include <asm/io.h>
#include <video/vga.h>

uint16_t *video_buffer = (uint16_t *) VGA_BUFFER;

int off_x, off_y;

static void move_cursor() {
    uint32_t off = get_offset();
    outb(VGA_CMD, 14);
    outb(VGA_DAT, off >> 8);
    outb(VGA_CMD, 15);
    outb(VGA_DAT, off);
}

static void print_backspace() {
    video_buffer[get_offset() - 1] = 0x20 | (WHITE_ON_BLACK << 8);
    off_x--;
    move_cursor();
}

static void scroll() {
    if (off_y >= VGA_HEIGHT) {
        int i;
        for (i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            video_buffer[i] = video_buffer[VGA_WIDTH + i];
        }

        for (i = (VGA_HEIGHT - 1 ) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            video_buffer[i] = 0x20 | (WHITE_ON_BLACK << 8);
        }
        off_y = VGA_HEIGHT - 1;
    }
}

int get_offset() {
    return off_y * VGA_WIDTH + off_x;
}

void putc(char c) {
    uint16_t *pos;

    if (c == '\b' && off_x) {
        print_backspace();
    } else if (c == '\n') {
        off_x = 0;
        off_y++;
    } else {
        pos = video_buffer + (get_offset());
        *pos = c | (WHITE_ON_BLACK << 8);
        off_x++;
    }

    if (off_x >= VGA_WIDTH) {
        off_x = 0;
        off_y++;
    }
    scroll();
    move_cursor();
}

void clear() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_buffer[i] = 0x20 | (WHITE_ON_BLACK << 8);
    }
    off_x = 0;
    off_y = 0;
    move_cursor();
}

void puts(char *string) {
    int i = 0;
    while (string[i]) {
        putc(string[i++]);
    }
}