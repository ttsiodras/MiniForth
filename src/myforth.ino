#include <stdint.h>
#include <string.h>

#include "miniforth.h"
#include "getline.h"
#include "helpers.h"

void setup()
{
    Serial.begin(115200); 
}

void loop()
{
    static char line[MAX_LINE_LENGTH];
    static int runBefore;
    static Forth miniforth;

    if (!runBefore) {
        runBefore = 1;
        miniforth.reset();
    }
    if (!get(line))
        exit(0);
    if (miniforth.parse_line(line, line + strlen(line)))
        Serial.print(F(" OK\n"));
}

#ifdef __x86_64__

int main()
{
    setup();
    while(1) {
        loop();
    }
}

SerialStub Serial;

#endif
