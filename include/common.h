#pragma once

#include <stdint.h>

// code macros
#define PACKED __attribute__((packed))
#define ASSERT(b) ((b) ? (void) 0 : printf("%s, %d, %s", __FILE__, __LINE__, #b))

#define NULL ((void *) 0)

#define VESA_MODE