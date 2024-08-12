#pragma once

#include <common.h>

void pit_install(uint32_t freq);

void pit_sleep(uint32_t ms);
uint32_t pit_get_ticks();