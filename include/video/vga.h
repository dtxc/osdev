#pragma once

#include <common.h>

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define WHITE_ON_BLACK 0x0F

void putc(char c);
int get_offset();
void puts(char *string);
void clear();