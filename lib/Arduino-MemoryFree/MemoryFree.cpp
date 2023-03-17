#include "MemoryFree.h"

/*
extern uint32_t _stack;
extern uint32_t _estack;
extern uint32_t _heap;
extern uint32_t _eheap;
*/
uint32_t freeMemory() {
  return freeStackMemory() + freeHeapMemory();
}
uint32_t freeStackMemory() {
  return 10000; //(uint32_t)(stackPointer() - stackEnd()); //Stack grows to the start of the mem
}
uint32_t freeHeapMemory() {
  return 10000; //(uint32_t)(heapEnd() - heapPointer()); //Heap grows to the end of the mem
}

uint8_t* stackStart() {
  return  0;
}
uint8_t* heapStart() {
  return 0;
}
uint8_t* stackEnd() {
  return 0;
}
uint8_t* heapEnd() {
  return 0;
}

uint8_t* stackPointer() {
  //char top;
  //return (uint32_t*)&top;
  register unsigned* stack_pointer;
  __asm__ volatile("mov %0, sp\n" : "=r" (stack_pointer) );
  return (uint8_t*)stack_pointer;
}
uint8_t* heapPointer() {
  return (uint8_t*)0;
}

void setCanaries() {
  uint32_t *stack_end = (uint32_t*)stackEnd();
  uint32_t *stackPtr = (uint32_t*)stackPointer();

  while(stack_end<stackPtr) {
    stack_end[0] = CANARY_STACK;
    stack_end += 1;
  }
  
  uint32_t *heap_end = (uint32_t*)heapEnd();
  uint32_t *heapPtr = (uint32_t*)heapPointer();

  heap_end -= 1; //sic!
  while(heap_end>heapPtr) {
    heap_end[0] = CANARY_HEAP;
    heap_end -= 1;
  }
}

bool testStackCanary() {
  uint32_t *stack_end = (uint32_t*)stackEnd();
  return (stack_end[0] == CANARY_STACK);
}
bool testHeapCanary() {
  uint32_t *heap_end = (uint32_t*)heapEnd();
  heap_end -= 1; //sic!
  return (heap_end[0] == CANARY_HEAP);
}

uint32_t countStackCanaries() {
  uint32_t canaries = 0;
  uint32_t *stackPtr = (uint32_t*)stackPointer();
  uint32_t *stack_end = (uint32_t*)stackEnd();

  while (stack_end[0] == CANARY_STACK && stack_end < stackPtr) {
    canaries++;
    stack_end += 1;
  }
  return canaries;
}
uint32_t countHeapCanaries() {
  uint32_t canaries = 0;
  uint32_t *heapPtr = (uint32_t*)heapPointer();
  uint32_t *heap_end = (uint32_t*)heapEnd();
  heap_end -= 1; //sic!

  while (heap_end[0] == CANARY_HEAP && heap_end > heapPtr) {
    canaries++;
    heap_end -= 1;
  }
  return canaries;
}

uint32_t getFirstStackCanary() {
  return (uint32_t)stackEnd()+(4*countStackCanaries());
}
uint32_t getFirstHeapCanary() {
  return (uint32_t)heapEnd()-(4*countHeapCanaries());
}