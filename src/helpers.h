#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "errors.h"

#define dprintf(fmt, ...) flash_printf(F(fmt), __VA_ARGS__)

void memory_info(unsigned freeListTotals);
void flash_printf(const __FlashStringHelper *fmt, ...);

#endif
