#pragma once

#include <common.h>

void memset(void *dst, uint8_t c, int n);
void memcpy(void *dst, void *src, int num);

int strcpy(char *dst, const char *src);
int strncpy(char *dst, const char *src, int n);
int strcmp(char *s, const char *s1);
int strncmp(char *s, const char *s1, int n);
int strlen(const char *s);

int itoa(char *dst, int n);
int int2hex(char *dst, uint32_t n);
int split_string(char *str, char delimiter, char ***tokens);
