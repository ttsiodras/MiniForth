#include <Arduino.h>

#include "defines.h"
#include "getline.h"

void get(char *cmd)
{
    int cmdIdx = 0;
    while(1) {
        int c = Serial.read();
        if (c!=-1) {
            if (c == 8 && cmdIdx>0) {
                Serial.print("\b \b");
                cmdIdx--;
            } else if (c == '\r') {
                cmd[cmdIdx & (MAX_LINE_LENGTH-1)] = '\0';
                break;
            } else if (isprint(c)) {
                Serial.print((char)c);
                cmd[cmdIdx++ & (MAX_LINE_LENGTH-1)] = (char)c;
            }
        }
    }
}

