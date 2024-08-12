#pragma once

#include <common.h>
#include <stdarg.h>

void vprintf(const char *s, va_list list);
void printf(const char *s, ...);