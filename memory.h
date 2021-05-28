#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define MEMORY_SIZE 0x10000

// I/O registers addr shortcuts

#define DIV     0xFF04

#define SCY     0xFF42
#define SCX     0xFF43
#define LY      0xFF44
#define LYC     0xFF45

#define IF      0xFF0F
#define IE      0xFFFF


uint8_t mem_get_byte(uint16_t addr);
void mem_set_byte(uint16_t addr, uint8_t value);
void mem_fill(uint16_t addr, uint8_t *data, uint16_t size);
void mem_init();

int dump_VRAM();

#endif