#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>


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


void cpu_reset_registers();


uint8_t cpu_get_A();
uint8_t cpu_get_B();
uint8_t cpu_get_C();
uint8_t cpu_get_D();
uint8_t cpu_get_E();
uint8_t cpu_get_F();
uint8_t cpu_get_H();
uint8_t cpu_get_L();

uint16_t cpu_get_AF();
uint16_t cpu_get_BC();
uint16_t cpu_get_DE();
uint16_t cpu_get_HL();

uint16_t cpu_get_SP();
uint16_t cpu_get_PC();

void cpu_set_A(uint8_t value);
void cpu_set_B(uint8_t value);
void cpu_set_C(uint8_t value);
void cpu_set_D(uint8_t value);
void cpu_set_E(uint8_t value);
void cpu_set_F(uint8_t value);
void cpu_set_H(uint8_t value);
void cpu_set_L(uint8_t value);

void cpu_set_AF(uint16_t value);
void cpu_set_BC(uint16_t value);
void cpu_set_DE(uint16_t value);
void cpu_set_HL(uint16_t value);

void cpu_set_SP(uint16_t value);
void cpu_set_PC(uint16_t value);

uint8_t cpu_exec_opcode(uint8_t opcode);

int cpu_set_flag(cpu_flag_name flag, cpu_flag_value value);

#endif