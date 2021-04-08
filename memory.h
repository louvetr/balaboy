#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define MEMORY_SIZE 0xFFFF

uint8_t mem_get_byte(uint16_t addr);
void mem_set_byte(uint16_t addr, uint8_t value);

#endif