#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define MEMORY_SIZE 0x10000
#define OAM_ADDR    0xFE00

#define BANK_SIZE_ROM   16384   //16kB
#define BANK_SIZE_RAM   2048    //2kB      

// I/O registers addr shortcuts

#define P1      0xFF00
#define DIV     0xFF04

#define LCDC    0xFF40
#define SCY     0xFF42
#define SCX     0xFF43
#define LY      0xFF44
#define LYC     0xFF45

#define IF      0xFF0F
#define IE      0xFFFF

typedef enum {
    TYPE_MBC0 = 0,
    TYPE_MBC1 = 1, 
    TYPE_MBC1_RAM = 2, 
    TYPE_MBC1_RAM_BATT = 3, 
} memory_cart_type;

typedef enum {
    MODE_MBC0,
    MODE_MBC1_16_8, 
    MODE_MBC1_4_32, 
} memory_cart_mode;


struct cartridge_info {
    uint8_t *mem;
    memory_cart_type type;
    memory_cart_mode mode;
    uint8_t ROM_bank_nb;
    uint8_t ROM_bank_active;
    uint8_t RAM_bank_nb;
    uint8_t RAM_bank_active;
    uint8_t RAM_banking_enable;
};


uint8_t mem_get_byte(uint16_t addr);
void mem_set_byte(uint16_t addr, uint8_t value);
void mem_fill(uint16_t addr, uint8_t *data, uint16_t size);
void mem_DIV_increment(uint8_t opcode_duration);
void mem_init();
int mem_load_rom(char* path);

int dump_VRAM();

#endif