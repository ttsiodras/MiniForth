#include <Arduino.h>

#include "defines.h"

#ifdef __x86_64

bool get(char *cmd)
{
    return NULL != fgets(cmd, MAX_LINE_LENGTH, stdin);
}

#else

bool get(char *cmd)
{
    int cmdIdx = 0;
    while(1) {
        int c = Serial.read();
        if (c!=-1) {
            if (c == 8 && cmdIdx>0) {
                Serial.print(F("\b \b"));
                cmdIdx--;
            } else if (c == '\r') {
                cmd[cmdIdx & (MAX_LINE_LENGTH-1)] = '\0';
                break;
            } else if (isprint(c)) {
                //Serial.print((char)c);
                if (cmdIdx<MAX_LINE_LENGTH)
                    cmd[cmdIdx++ & (MAX_LINE_LENGTH-1)] = (char)c;
                else
                    Serial.print(F("\b \b"));
            }
        }
    }
    return true;
}

#endif
