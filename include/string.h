#pragma once

#include <common.h>

void *memset(void *dst, uint8_t c, int n);

int strcpy(char *dst, const char *src);
int strcmp(char *s, const char *s1);
int strncmp(char *s, const char *s1, int n);
int strlen(const char *s);

int itoa(char *dst, int n);
int int2hex(char *dst, uint32_t n);