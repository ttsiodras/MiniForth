#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "errors.h"

#define dprintf(fmt, ...) flash_printf(F(fmt), __VA_ARGS__)

void memory_info();
void flash_printf(const __FlashStringHelper *fmt, ...);
bool get(char *cmd);

#endif
