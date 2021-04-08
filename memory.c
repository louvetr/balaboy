
#include "memory.h"

// declare locally the memory array
static uint8_t memory[MEMORY_SIZE];

uint8_t mem_get_byte(uint16_t addr) {return memory[addr]; }
void mem_set_byte(uint16_t addr, uint8_t value) { memory[addr] = value; }