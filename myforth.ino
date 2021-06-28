#include <stdint.h>
#include <string.h>

#include "forth_types.h"
#include "getline.h"

Forth *miniforth;

void setup()
{
    Serial.begin(9600); 
    dprintf("\n\n============\n");
    dprintf("TTSIOD Forth\n");
    dprintf("============\n");
}

void loop()
{
    static char line[MAX_LINE_LENGTH];

    if (!miniforth) {
        miniforth = new Forth();
        DASSERT(miniforth);
    }
    get(line);
    dprintf(" ");
    if (miniforth->parse_line(line, line + strlen(line)))
        dprintf(" OK\n");
}
