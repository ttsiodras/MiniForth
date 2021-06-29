#include "mini_stl.h"
#include "defines.h"

char Pool::pool_data[POOL_SIZE];
size_t Pool::pool_offset = 0;
