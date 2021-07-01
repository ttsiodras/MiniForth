#include <Arduino.h>

#include "defines.h"

#ifdef __x86_64

#include <string.h>

bool get(char *cmd)
{
    printf("> ");
    bool ret = NULL != fgets(cmd, MAX_LINE_LENGTH, stdin);
    if (strlen(cmd) >= MAX_LINE_LENGTH-1) 
        puts("############################################## Too lengthy line!");
    return ret;
}

#else

bool get(char *cmd)
{
    int cmdIdx = 0;
    Serial.print("> ");
    while(1) {
        int c = Serial.read();
        if (c!=-1) {
            if (c == 8 && cmdIdx>0) {
                Serial.print(F(" \b"));
                cmdIdx--;
            } else if (c == '\r') {
                cmd[cmdIdx] = '\0';
                break;
            } else if (isprint(c)) {
                //Serial.print((char)c);
                if (cmdIdx<MAX_LINE_LENGTH)
                    cmd[cmdIdx++] = (char)c;
                else
                    Serial.print(F("\b \b"));
            }
        }
    }
    return true;
}

#endif
