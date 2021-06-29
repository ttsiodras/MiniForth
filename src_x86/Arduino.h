#ifndef __ARDUINO_X86__
#define __ARDUINO_X86__

#ifdef __x86_64

#include <stdio.h>
#include <stdlib.h>

#define F(x) x

class SerialStub {
public:
    void print(const char *msg) {
        printf(msg);
    }

    void print(int intVal) {
        printf("%d", intVal);
    }

    void print(long unsigned int intVal) {
        printf("%lu", intVal);
    }

    void println(long unsigned int intVal) {
        printf("%lu\n", intVal);
    }

    void println(const char *msg) {
        puts(msg);
    }

    void flush() {
        fflush(stdout);
    }

    void begin(int) {}
};

extern SerialStub Serial;

#endif

#endif
