#include <stdint.h>
#include <string.h>

#include "miniforth.h"
#include "helpers.h"

void setup()
{
    Serial.begin(115200); 
    Serial.println(F("\n\n============"));
    Serial.println(F("TTSIOD Forth"));
    Serial.println(F("============"));
}

void loop()
{
    static char line[MAX_LINE_LENGTH];
    static int runBefore;
    static Forth miniforth;

    if (!runBefore) {
        runBefore = 1;
        memory_info();
        Serial.println(F("Type 'wrd' (without the quotes) to see available words."));
    }
    if (!get(line))
        exit(0);
    Serial.print(F(" "));
    if (miniforth.parse_line(line, line + strlen(line)))
        Serial.print(F(" OK\n"));
}

#ifdef __x86_64

int main()
{
    setup();
    while(1) {
        loop();
    }
}

SerialStub Serial;

#endif
