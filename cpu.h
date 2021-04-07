#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

struct cpu_registers {
    uint8_t A;
    uint8_t F;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint16_t SP;    // stack pointer
    uint16_t PC;    // program counter

    //uint16_t BC;
};

typedef enum {
    FLAG_ZERO,
    FLAG_SUB, 
    FLAG_HALF_CARRY,
    FLAG_CARRY,
} cpu_flag_name;

typedef enum {
    FALSE = 0,
    TRUE = 1,
} cpu_flag_value;

uint16_t get_AF(struct cpu_registers *regs);
uint16_t get_BC(struct cpu_registers *regs);
uint16_t get_DE(struct cpu_registers *regs);
uint16_t get_HL(struct cpu_registers *regs);

void set_AF(struct cpu_registers *regs, uint16_t value);
void set_BC(struct cpu_registers *regs, uint16_t value);
void set_DE(struct cpu_registers *regs, uint16_t value);
void set_HL(struct cpu_registers *regs, uint16_t value);

uint8_t exec_opcode(uint8_t opcode, struct cpu_registers *regs, uint8_t *mem);

int set_cpu_flag(struct cpu_registers *regs, cpu_flag_name flag,  cpu_flag_value value);

#endif