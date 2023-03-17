// memoryFree header
// From http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720/15
// ...written by user "mem".

#ifndef	MEMORY_FREE_H
#define MEMORY_FREE_H
#include <stdint.h>

const uint32_t MEMORY_START = 0x20004000;
const uint32_t MEMORY_END = MEMORY_START + 0x3C000;
const uint32_t CANARY_STACK = 0xE0BEAD0B; //0x0BADBEE0;
const uint32_t CANARY_HEAP =  0xE0BE4D0B; //0x0B4DBEE0;

uint32_t freeMemory();
uint32_t freeStackMemory();
uint32_t freeHeapMemory();

uint8_t* stackStart();
uint8_t* heapStart();

uint8_t* stackEnd();
uint8_t* heapEnd();

uint8_t* stackPointer();
uint8_t* heapPointer();

void setCanaries();
bool testStackCanary();
bool testHeapCanary();
uint32_t countStackCanaries();
uint32_t countHeapCanaries();
uint32_t getFirstStackCanary();
uint32_t getFirstHeapCanary();

#endif
