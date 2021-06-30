#ifndef __DEFINES_H__
#define __DEFINES_H__

// Including null terminators
#define MAX_LINE_LENGTH 64
#define MEMORY_SIZE 4

#ifndef __x86_64

// The Pool size that hosts our own heap (out of the 2K SRAM).
// Defined as whatever is left once we reserve 250 bytes
// for the run-time AVR stack (i.e. the CPU stack).
// We subtract 250 from the free space reported by avr-size
// after the build; andcCurrent state is 1350 bytes (of the 2K)
// reserved for the dictionary and Forth run-time stack
// (i.e. CompiledNodes and StackNodes).
//
//    Sketch uses 11008 bytes (34%) of program storage space.
//    Maximum is 32256 bytes.  Global variables use 1799 bytes (87%)
//    of dynamic memory, leaving 249 bytes for local variables.
//    Maximum is 2048 bytes.

#define POOL_SIZE 1050

#else

// For x86 testing, multiply by 2 - to accomodata the 32-bit
// integers (4 bytes instead of 2).
#define POOL_SIZE 1050*2

#endif

#endif
