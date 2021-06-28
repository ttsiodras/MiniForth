#ifndef __DEFINES_H__
#define __DEFINES_H__

#include "helpers.h"

// Including null terminators
#define MAX_NAME_LENGTH 20
#define MAX_LINE_LENGTH 32
#define MAX_DICT_ENTRIES 10
#define MAX_PHRASE_ENTIES 5
#define MEMORY_SIZE 4

///////////////////////////////////////////////////
// Helpers for error messages and error propagation

#define SAFE_STRCPY(dest, src) {               \
    strncpy(&dest[0], src, sizeof(dest)-1);    \
    dest[sizeof(dest)-1] = '\0';               \
}

#endif
