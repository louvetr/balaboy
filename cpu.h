#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>


#define INT_VBLANK          0x01
#define INT_VBLANK_ADDR     0x40
#define INT_LCDC            0x02
#define INT_LCDC_ADDR       0x48
#define INT_TIMER           0x04
#define INT_TIMER_ADDR      0x50
#define INT_SERIAL          0x08
#define INT_SERIAL_ADDR     0x58
#define INT_JOYPAD          0x10
#define INT_JOYPAD_ADDR     0x60

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

void SP_push(uint16_t val);

uint8_t cpu_exec_opcode(uint8_t *opcode_length, uint8_t *opcode_duration);

cpu_flag_value cpu_get_flag(cpu_flag_name flag);
int cpu_set_flag(cpu_flag_name flag, cpu_flag_value value);

uint8_t cpu_get_interrupts_enabled();
void cpu_set_interrupts_enabled(uint8_t val);

void cpu_init();

#endif