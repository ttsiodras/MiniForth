#ifndef __DEFINES_H__
#define __DEFINES_H__

// Including null terminators
#define MAX_LINE_LENGTH 80
#define MAX_NATIVE_COMMAND_LENGTH 6

#define MEMORY_SIZE 4

#ifndef __x86_64__

// The Pool and Stack size that host our data.
// For the stack size, the value depends on your program
// and how "deep" it goes. For my extensive test scenario
// that includes FizzBuzz, I went with 280 bytes.
//
// The Pool is defined as whatever is left once we reserve 280 bytes
// for the run-time AVR stack (i.e. the CPU stack). We therefore
// subtract 280 from the free space reported after the build,
// and dimension the POOL_SIZE with whatever is left.
// 
// For example - this is what arduino-builder emits as I am
// writing this comment:
//
//   Global variables use 1640 bytes (80%) of dynamic memory,
//   leaving 408 bytes for local variables. Maximum is 2048 bytes.
//
// That means we have extra space for 408-280 = 128 bytes;
// so we decrease our FORTH_GLOBALS by that amount, and...
//
//   Global variables use 1768 bytes (86%) of dynamic memory,
//   leaving 280 bytes for local variables. Maximum is 2048 bytes.
//
// ...now we are told we have exactly 280 bytes left at run-time;
// which is what we have reserved for the CPU stack.
//
// In effect, our pool increase changes the "Global variables use"
// from 1640 to 1640+128 = 1768 bytes. Adding 280 for stack,
// we have 2048 bytes - our total SRAM.
//
// In this configuration, we therefore have...
//
// - 280 bytes (for our CPU stack)
// - and 1388 bytes (for our FORTH stacks)
//
// Not bad! Lots of FORTH code can be written in 1.4K,
// so we make good use of our 2K of SRAM :-)

#define ATMEGA328_MEMORY   2048
#define STACK_SIZE         280
#define FORTH_GLOBALS      380
#define POOL_SIZE (ATMEGA328_MEMORY - STACK_SIZE - FORTH_GLOBALS)

#else

// For x86 testing, just use 4K. Pointers and integers are much
// bigger, so this is still a good test.
#define POOL_SIZE 4096

#define PROGMEM
#define __FlashStringHelper char
#define strcasecmp_P strcasecmp
#define strncpy_P strncpy
#define strlen_P strlen
#define PGM_P const char *
#define pgm_read_word_near(x) (*(x))

#endif

#endif
