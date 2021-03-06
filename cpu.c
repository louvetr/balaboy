#include <stdlib.h>

#include "cpu.h"
#include "memory.h"

/////////////////////////////////////////////////////////////////////////////////////
// defintions and local stuff
/////////////////////////////////////////////////////////////////////////////////////

// define locally the CPU register structure
struct cpu_registers {
	uint8_t A;
	uint8_t F;
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint16_t SP; // stack pointer
	uint16_t PC; // program counter
};

// declare locally an instance of the register structure.
// This struct var has to be reach externally via getter and setter funcions
static struct cpu_registers regs;

static uint8_t	cpu_interrupts_enabled = 1;

static uint8_t cpu_exec_opcode_CB(uint8_t opcode);

/////////////////////////////////////////////////////////////////////////////////////
// private functions
/////////////////////////////////////////////////////////////////////////////////////

static void INC_u8(uint8_t *ptr)
{
	(*ptr)++;
	cpu_set_flag(FLAG_ZERO, *ptr == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, *ptr == 0 ? TRUE : FALSE);
}

static void DEC_u8(uint8_t *ptr)
{
	(*ptr)--;
	cpu_set_flag(FLAG_ZERO, *ptr == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, TRUE);
	cpu_set_flag(FLAG_HALF_CARRY, *ptr == 255 ? TRUE : FALSE);
	// TODO: check HC flag
}

static void LD_mem_u8(uint16_t addr, uint8_t src)
{
	/*if(addr == 0x9bff)
		printf("test\n");*/
	mem_set_byte(addr, src);
}

static void LD_reg_u8(uint8_t *reg_u8, uint8_t src)
{
	*reg_u8 = src;
}

static void LD_mem_u16(uint16_t addr, uint16_t src)
{
	mem_set_byte(addr, src & 0x00FF);
	mem_set_byte(addr + 1, src >> 8);
}

static void ADD_to_A(uint8_t val_to_add)
{
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY,
		     (regs.A & 0x0F) + (val_to_add & 0x0F) > 0x0F ? TRUE :
								    FALSE);
	cpu_set_flag(FLAG_CARRY,
		     (uint16_t)regs.A + (uint16_t)val_to_add > 0x00FF ? TRUE :
									FALSE);
	regs.A += val_to_add;
	cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
	// TODO: check HC flag
}

static void ADC_to_A(uint8_t val_to_add)
{
	cpu_set_flag(FLAG_SUB, FALSE);
	uint8_t tmp_hc = (regs.A & 0x0F) + (val_to_add & 0x0F) +
						 cpu_get_flag(FLAG_CARRY) >
					 0x0F ?
				 TRUE :
				 FALSE;
	uint8_t tmp_h =
		(uint16_t)regs.A + (uint16_t)val_to_add +
					(uint16_t)cpu_get_flag(FLAG_CARRY) >
				0x00FF ?
			TRUE :
			FALSE;
	regs.A = regs.A + val_to_add + cpu_get_flag(FLAG_CARRY);
	cpu_set_flag(FLAG_HALF_CARRY, tmp_hc);
	cpu_set_flag(FLAG_CARRY, tmp_h);
	cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
	// TODO: check HC flag
}

static void SUB_to_A(uint8_t val_to_sub)
{
	cpu_set_flag(FLAG_SUB, TRUE);
	cpu_set_flag(FLAG_HALF_CARRY,
		     (regs.A & 0x0F) < (val_to_sub & 0x0F) ? TRUE : FALSE);
	cpu_set_flag(FLAG_CARRY, regs.A < val_to_sub ? TRUE : FALSE);
	//printf("[SUB] 0x%x - 0x%x = (uint8_t)  0x%x\n", regs.A, val_to_sub, (uint8_t)(regs.A - val_to_sub));
	regs.A -= val_to_sub;
	cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
}

static void SBC_to_A(uint8_t val_to_sub)
{
	cpu_set_flag(FLAG_SUB, TRUE);
	uint8_t tmp_hc = (regs.A & 0x0F) < ((val_to_sub & 0x0F) +
					    cpu_get_flag(FLAG_CARRY)) ?
				 TRUE :
				 FALSE;
	uint8_t tmp_h =
		regs.A < (val_to_sub + cpu_get_flag(FLAG_CARRY)) ? TRUE : FALSE;
	//printf("[SUB] 0x%x - 0x%x - 0x%x = (uint8_t)  0x%x\n", regs.A, val_to_sub, cpu_get_flag(FLAG_CARRY), (uint8_t)(regs.A - val_to_sub));
	regs.A = regs.A - val_to_sub - cpu_get_flag(FLAG_CARRY);
	cpu_set_flag(FLAG_HALF_CARRY, tmp_hc);
	cpu_set_flag(FLAG_CARRY, tmp_h);
	cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
}

static void AND_with_A(uint8_t val)
{
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, TRUE);
	cpu_set_flag(FLAG_CARRY, FALSE);
	regs.A &= val;
	cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
}

static void XOR_with_A(uint8_t val)
{
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, FALSE);
	//regs.A ^= val;
	//printf(" A(0x%x) XOR val(0x%x) = 0x%x\n", regs.A, val, regs.A ^ val);
	regs.A = regs.A ^ val;
	cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
}

static void OR_with_A(uint8_t val)
{
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, FALSE);
	regs.A |= val;
	cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
}

static void CP_with_A(uint8_t val)
{
	cpu_set_flag(FLAG_SUB, TRUE);
	cpu_set_flag(FLAG_HALF_CARRY, (regs.A & 0x0F) < (val & 0x0F) ? TRUE : FALSE);
	cpu_set_flag(FLAG_CARRY, regs.A < val ? TRUE : FALSE);
	cpu_set_flag(FLAG_ZERO, regs.A == val ? TRUE : FALSE);
}

static uint16_t SP_pop(){
	uint16_t tmp_u16 = mem_get_byte(regs.SP + 1) << 8 |
	       			   mem_get_byte(regs.SP);
	cpu_set_SP(cpu_get_SP()+2);
	return tmp_u16;
}

/*static*/ void SP_push(uint16_t val){
	cpu_set_SP(cpu_get_SP()-2);
	LD_mem_u16(cpu_get_SP(), val);
}

static void RLC(uint8_t *dst){
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, *dst >> 7);
	*dst = *dst << 1 | *dst >> 7;
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
}

static void RRC(uint8_t *dst){
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, *dst & 0x01);
	*dst = *dst >> 1 | *dst << 7;
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
}

static void RL(uint8_t *dst){
	uint8_t tmp_carry = *dst >> 7;
	*dst = (*dst << 1) | cpu_get_flag(FLAG_CARRY);
	cpu_set_flag(FLAG_CARRY, tmp_carry);
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
}

static void RR(uint8_t *dst){
	uint8_t tmp_carry = *dst & 0x01;
	*dst = *dst >> 1 | cpu_get_flag(FLAG_CARRY) << 7;
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, tmp_carry);
}

static void SLA(uint8_t *dst){
	uint8_t tmp_carry = *dst >> 7;
	*dst = (*dst << 1);
	cpu_set_flag(FLAG_CARRY, tmp_carry);
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
}

static void SRA(uint8_t *dst){
	uint8_t tmp_carry = *dst & 0x01;
	*dst = (*dst >> 1) | (*dst & 0x80);
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, tmp_carry);
}

static void SRL(uint8_t *dst){
	uint8_t tmp_carry = *dst & 0x01;
	*dst = (*dst >> 1);
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, tmp_carry);
}

static void SWAP(uint8_t *dst){
	uint8_t p1 = (*dst & 0x0F) << 4;
	uint8_t p2 = *dst >> 4;
	*dst = p1 | p2;
	cpu_set_flag(FLAG_ZERO, *dst == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, FALSE);
	cpu_set_flag(FLAG_CARRY, FALSE);
}


static void BIT(uint8_t bit2test, uint8_t *p_reg) {
	uint8_t val = *p_reg & (1 << bit2test);
	//printf("[BIT] %d = 0x%x & (1 << %d) = 0x%x & 0x%x\n", val, *p_reg, bit2test, *p_reg, (1 << bit2test));
	cpu_set_flag(FLAG_ZERO, val == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, TRUE);
}

static void RES(uint8_t bit2test, uint8_t *p_reg) {
	*p_reg = *p_reg & ~(1 << bit2test);
}

static void SET(uint8_t bit2test, uint8_t *p_reg) {
	*p_reg = *p_reg | (1 << bit2test);
}

/////////////////////////////////////////////////////////////////////////////////////
// public functions
/////////////////////////////////////////////////////////////////////////////////////

uint8_t cpu_get_interrupts_enabled()
{
	return cpu_interrupts_enabled;
}

void cpu_set_interrupts_enabled(uint8_t val)
{
	cpu_interrupts_enabled = val;
}

void cpu_reset_registers()
{
	memset(&regs, 0, sizeof(struct cpu_registers));
}

// getter of 8 bits registers
uint8_t cpu_get_A()
{
	return regs.A;
}
uint8_t cpu_get_B()
{
	return regs.B;
}
uint8_t cpu_get_C()
{
	return regs.C;
}
uint8_t cpu_get_D()
{
	return regs.D;
}
uint8_t cpu_get_E()
{
	return regs.E;
}
uint8_t cpu_get_F()
{
	return regs.F;
}
uint8_t cpu_get_H()
{
	return regs.H;
}
uint8_t cpu_get_L()
{
	return regs.L;
}

// setter of 8 bits registers
void cpu_set_A(uint8_t value)
{
	regs.A = value;
}
void cpu_set_B(uint8_t value)
{
	regs.B = value;
}
void cpu_set_C(uint8_t value)
{
	regs.C = value;
}
void cpu_set_D(uint8_t value)
{
	regs.D = value;
}
void cpu_set_E(uint8_t value)
{
	regs.E = value;
}
void cpu_set_F(uint8_t value)
{
	regs.F = value;
}
void cpu_set_H(uint8_t value)
{
	regs.H = value;
}
void cpu_set_L(uint8_t value)
{
	regs.L = value;
}

// getter of 16 bits registers
uint16_t cpu_get_AF()
{
	return regs.A << 8 | regs.F;
}
uint16_t cpu_get_BC()
{
	return regs.B << 8 | regs.C;
}
uint16_t cpu_get_DE()
{
	return regs.D << 8 | regs.E;
}
uint16_t cpu_get_HL()
{
	return regs.H << 8 | regs.L;
}

// getter of 16 bits registers
void cpu_set_AF(uint16_t value)
{
	regs.A = (uint8_t)(value >> 8);
	regs.F = (uint8_t)(value & 0x00FF);
}
void cpu_set_BC(uint16_t value)
{
	regs.B = (uint8_t)(value >> 8);
	regs.C = (uint8_t)(value & 0x00FF);
}
void cpu_set_DE(uint16_t value)
{
	regs.D = (uint8_t)(value >> 8);
	regs.E = (uint8_t)(value & 0x00FF);
}
void cpu_set_HL(uint16_t value)
{
	regs.H = (uint8_t)(value >> 8);
	regs.L = (uint8_t)(value & 0x00FF);
}

// getter of 16 bits registers
uint16_t cpu_get_SP()
{
	return regs.SP;
}
uint16_t cpu_get_PC()
{
	return regs.PC;
}

// setter of 16 bits registers
void cpu_set_SP(uint16_t value)
{
	regs.SP = value;
}
void cpu_set_PC(uint16_t value)
{
	regs.PC = value;
}

int cpu_set_flag(cpu_flag_name flag, cpu_flag_value value)
{
	switch (flag) {
	case FLAG_ZERO:
		regs.F = regs.F & 0x7F | value << 7;
		break;
	case FLAG_SUB:
		regs.F = regs.F & 0xBF | value << 6;
		break;
	case FLAG_HALF_CARRY:
		regs.F = regs.F & 0xDF | value << 5;
		break;
	case FLAG_CARRY:
		regs.F = regs.F & 0xEF | value << 4;
		break;
	default:
		printf("[ERROR][%s:%d] invalid flag\n", __func__, __LINE__);
	}

	return 0;
}

cpu_flag_value cpu_get_flag(cpu_flag_name flag)
{
	cpu_flag_value ret = FALSE;
	switch (flag) {
	case FLAG_ZERO:
		ret = regs.F & 0x80 ? TRUE : FALSE;
		break;
	case FLAG_SUB:
		ret = regs.F & 0x40 ? TRUE : FALSE;
		break;
	case FLAG_HALF_CARRY:
		ret = regs.F & 0x20 ? TRUE : FALSE;
		break;
	case FLAG_CARRY:
		ret = regs.F & 0x10 ? TRUE : FALSE;
		break;
	default:
		printf("[ERROR][%s:%d] invalid flag\n", __func__, __LINE__);
	}
}

uint8_t cpu_exec_opcode(uint8_t *opcode_length, uint8_t *opcode_duration)
{
	uint8_t length = 0; // length in byte
	uint8_t duration = 0; // duration in clock cycles
	uint8_t opcode = mem_get_byte(regs.PC);
	uint8_t u8 = mem_get_byte(regs.PC + 1);

	// Get the u16 value after opcode even if not needed.
	// Done before the switch per pure laziness (and to avoid possible error since we have 250+ case)

	// TODO: WARNING: check u8 LSB is OK for all u16 used by opcodes
	uint16_t u16 = mem_get_byte(regs.PC + 2) << 8 |
		       u8; // OP B1 B2 => B1 is LSB, B2 is MSB

	uint32_t u32 = 0;
	int8_t r8;
	uint8_t tmp_carry;
	uint8_t add_lg = 1;

	switch (opcode) {
	// 0x0X ////////////////////////////////////////////////////////////////
	case 0x00: // NOP
		length = 1;
		duration = 4;
		break;

	case 0x01: // LD BC,d16
		length = 3;
		duration = 12;
		cpu_set_BC(u16);
		break;

	case 0x02: // LD (BC),A
		length = 1;
		duration = 8;
		LD_mem_u8(cpu_get_BC(), regs.A);
		break;

	case 0x03: // INC BC
		length = 1;
		duration = 8;
		cpu_set_BC(cpu_get_BC() + 1);
		break;

	case 0x04: // INC B
		length = 1;
		duration = 4;
		INC_u8(&regs.B);
		break;

	case 0x05: // DEC B
		length = 1;
		duration = 4;
		DEC_u8(&regs.B);
		break;

	case 0x06: // LD B,d8
		length = 2;
		duration = 8;
		LD_reg_u8(&regs.B, mem_get_byte(regs.PC + 1));
		break;

	case 0x07: // RLC A
		length = 1;
		duration = 4;
		RLC(&regs.A);
		break;

	case 0x08: // LD (a16),SP
		length = 3;
		duration = 20;
		LD_mem_u16(u16, regs.SP);
		break;

	case 0x09: // ADD HL,BC
		length = 1;
		duration = 8;
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY,
			     regs.L + regs.C > 0xFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY,
			     (uint32_t)cpu_get_HL() + (uint32_t)cpu_get_BC() >
					     0xFFFF ?
				     TRUE :
				     FALSE);
		cpu_set_HL(cpu_get_HL() + cpu_get_BC());
		break;

	case 0x0A: // LD A,(BC)
		length = 1;
		duration = 8;
		LD_reg_u8(&regs.A, mem_get_byte(cpu_get_BC()));
		break;

	case 0x0B: // DEC BC
		length = 1;
		duration = 8;
		cpu_set_BC(cpu_get_BC() - 1);
		break;

	case 0x0C: // INC C
		length = 1;
		duration = 4;
		INC_u8(&regs.C);
		break;

	case 0x0D: // DEC C
		length = 1;
		duration = 4;
		DEC_u8(&regs.C);
		break;

	case 0x0E: // LD C,d8
		length = 2;
		duration = 8;
		LD_reg_u8(&regs.C, mem_get_byte(regs.PC + 1));
		break;

	case 0x0F: // RRC A
		length = 1;
		duration = 4;
		RRC(&regs.A);
		break;

	// 0x1X ////////////////////////////////////////////////////////////////
	case 0x10: // STOP
		length = 2;
		duration = 8;
		// TODO: perform stop action
		break;

	case 0x11: // LD DE,d16
		length = 3;
		duration = 12;
		cpu_set_DE(u16);
		break;

	case 0x12: // LD (DE),A
		length = 1;
		duration = 8;
		LD_mem_u8(cpu_get_DE(), regs.A);
		break;

	case 0x13: // INC DE
		length = 1;
		duration = 8;
		cpu_set_DE(cpu_get_DE() + 1);
		break;

	case 0x14: // INC D
		length = 1;
		duration = 4;
		INC_u8(&regs.D);
		break;

	case 0x15: // DEC D
		length = 1;
		duration = 4;
		DEC_u8(&regs.D);
		break;

	case 0x16: // LD D,d8
		length = 2;
		duration = 8;
		LD_reg_u8(&regs.D, mem_get_byte(regs.PC + 1));
		break;

	case 0x17: // RL A
		length = 1;
		duration = 4;
		RL(&regs.A);
		break;

	case 0x18: // JR r8
		length = 2;
		duration = 12;
		int8_t i8 = (int8_t)mem_get_byte(regs.PC + 1);
		/*printf("[cpu.c] regs.PC + i8 = 0x%x = 0x%x + %d (0x%x)\n",
		       regs.PC + i8, regs.PC, i8, mem_get_byte(regs.PC + 1));
		printf("[cpu.c] int16_t (int16_t)regs.PC + (int16_t)i8 = %d + %d = %d\n",
		       (int16_t)regs.PC + (int16_t)i8, (int16_t)regs.PC,
		       (int16_t)i8);*/
		regs.PC = (uint16_t)(
			(int16_t)regs.PC +
			(int16_t)i8); // TODO: check if final PC value is right
		//printf("[cpu.c] regs.PC = 0x%x\n", regs.PC);
		break;

	case 0x19: // ADD HL,DE
		length = 1;
		duration = 8;
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY,
			     regs.L + regs.E > 0xFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY,
			     (uint32_t)cpu_get_HL() + (uint32_t)cpu_get_DE() >
					     0xFFFF ?
				     TRUE :
				     FALSE);
		cpu_set_HL(cpu_get_HL() + cpu_get_DE());
		break;

	case 0x1A: // LD A,(DE)
		length = 1;
		duration = 8;
		LD_reg_u8(&regs.A, mem_get_byte(cpu_get_DE()));
		break;

	case 0x1B: // DEC DE
		length = 1;
		duration = 8;
		cpu_set_DE(cpu_get_DE() - 1);
		break;

	case 0x1C: // INC E
		length = 1;
		duration = 4;
		INC_u8(&regs.E);
		break;

	case 0x1D: // DEC E
		length = 1;
		duration = 4;
		DEC_u8(&regs.E);
		break;

	case 0x1E: // LD E,d8
		length = 2;
		duration = 8;
		LD_reg_u8(&regs.E, mem_get_byte(regs.PC + 1));
		break;

	case 0x1F: // RR A
		length = 1;
		duration = 4;
		RR(&regs.A);
		break;

	// 0x2X ////////////////////////////////////////////////////////////////
	case 0x20: // JR NZ,r8
		length = 2;

		if (cpu_get_flag(FLAG_ZERO)) {
			duration = 8;
		} else {
			duration = 12;
			int8_t i8 = (int8_t)mem_get_byte(regs.PC + 1);
			regs.PC = (uint16_t)(
				(int16_t)regs.PC +
				(int16_t)i8); // TODO: check if final PC value is right
			regs.PC += 2;
			add_lg = 0;
		}
		break;

	case 0x21: // LD HL,d16
		length = 3;
		duration = 12;
		cpu_set_HL(u16);
		break;

	case 0x22: // LD (HL+),A
		length = 1;
		duration = 8;
		LD_mem_u8(cpu_get_HL(), regs.A);
		cpu_set_HL(cpu_get_HL() + 1);
		break;

	case 0x23: // INC HL
		length = 1;
		duration = 8;
		cpu_set_HL(cpu_get_HL() + 1);
		break;

	case 0x24: // INC H
		length = 1;
		duration = 4;
		INC_u8(&regs.H);
		break;

	case 0x25: // DEC H
		length = 1;
		duration = 4;
		DEC_u8(&regs.H);
		break;

	case 0x26: // LD H,d8
		length = 2;
		duration = 8;
		LD_reg_u8(&regs.H, mem_get_byte(regs.PC + 1));
		break;

	case 0x27: // DAA
		length = 1;
		duration = 4;
		uint8_t D1 = regs.A >> 4;
		uint8_t D2 = regs.A & 0x0F;
		if (cpu_get_flag(FLAG_SUB)) {
			if (cpu_get_flag(FLAG_SUB) | D2 > 9)
				D2 -= 6;
			if (cpu_get_flag(FLAG_CARRY))
				D1 -= 6;
			if (D1 > 9) {
				D1 -= 6;
				cpu_set_flag(FLAG_CARRY, TRUE);
			}
		} else {
			if (cpu_get_flag(FLAG_HALF_CARRY))
				D2 += 6;
			if (cpu_get_flag(FLAG_CARRY))
				D1 += 6;
			if (D2 > 9) {
				D2 -= 10;
				D1++;
			}
			if (D1 > 9) {
				D1 -= 10;
				cpu_set_flag(FLAG_CARRY, TRUE);
			}
		}
		regs.A = ((D1 << 4) & 0xF0) | (D2 & 0x0F);
		cpu_set_flag(FLAG_ZERO, (regs.A == 0));
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
		break;

	case 0x28: // JR Z,r8
		length = 2;

		if (!cpu_get_flag(FLAG_ZERO)) {
			duration = 8;
		} else {
			duration = 12;
			int8_t i8 = (int8_t)mem_get_byte(regs.PC + 1);
			regs.PC = (uint16_t)(
				(int16_t)regs.PC +
				(int16_t)i8); // TODO: check if final PC value is right
			add_lg = 0;
			regs.PC += 2;
		}
		break;

	case 0x29: // ADD HL,HL // TODO
		length = 1;
		duration = 8;
		cpu_set_flag(FLAG_SUB, FALSE);
		// TODO: check if HC flag is managed correctly
		cpu_set_flag(FLAG_HALF_CARRY,
			     regs.L + regs.L > 0xFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY,
			     (uint32_t)cpu_get_HL() + (uint32_t)cpu_get_HL() >
					     0xFFFF ?
				     TRUE :
				     FALSE);
		cpu_set_HL(cpu_get_HL() + cpu_get_HL());
		break;

	case 0x2A: // LD A,(HL+)
		length = 1;
		duration = 8;
		LD_reg_u8(&regs.A, mem_get_byte(cpu_get_HL()));
		cpu_set_HL(cpu_get_HL() + 1);
		break;

	case 0x2B: // DEC HL
		length = 1;
		duration = 8;
		cpu_set_HL(cpu_get_HL() - 1);
		break;

	case 0x2C: // INC L
		length = 1;
		duration = 4;
		INC_u8(&regs.L);
		break;

	case 0x2D: // DEC L
		length = 1;
		duration = 4;
		DEC_u8(&regs.L);
		break;

	case 0x2E: // LD L,d8
		length = 2;
		duration = 8;
		LD_reg_u8(&regs.L, mem_get_byte(regs.PC + 1));
		break;

	case 0x2F: // CP L
		length = 1;
		duration = 4;
		regs.A = ~regs.A;
		cpu_set_flag(FLAG_SUB, TRUE);
		cpu_set_flag(FLAG_HALF_CARRY, TRUE);
		break;

	// 0x3X ////////////////////////////////////////////////////////////////
	case 0x30: // JR NC,r8
		length = 2;

		if (cpu_get_flag(FLAG_CARRY)) {
			duration = 8;
		} else {
			duration = 12;
			int8_t i8 = (int8_t)mem_get_byte(regs.PC + 1);
			regs.PC = (uint16_t)(
				(int16_t)regs.PC +
				(int16_t)i8); // TODO: check if final PC value is right
			add_lg = 0;
			regs.PC += 2;
		}
		break;

	case 0x31: // LD SP,d16
		length = 3;
		duration = 12;
		cpu_set_SP(u16);
		break;

	case 0x32: // LD (HL-),A
		length = 1;
		duration = 8;
		LD_mem_u8(cpu_get_HL(), regs.A);
		cpu_set_HL(cpu_get_HL() - 1);
		break;

	case 0x33: // INC SP
		length = 1;
		duration = 8;
		cpu_set_SP(cpu_get_SP() + 1);
		break;

	case 0x34: // INC (HL)
		length = 1;
		duration = 12;
		mem_set_byte(cpu_get_HL(), mem_get_byte(cpu_get_HL()) + 1);
		cpu_set_flag(FLAG_ZERO,
			     mem_get_byte(cpu_get_HL()) == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY,
			     mem_get_byte(cpu_get_HL()) == 0 ? TRUE : FALSE);
		break;

	case 0x35: // DEC (HL)
		length = 1;
		duration = 12;
		mem_set_byte(cpu_get_HL(), mem_get_byte(cpu_get_HL()) - 1);
		cpu_set_flag(FLAG_ZERO,
			     mem_get_byte(cpu_get_HL()) == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY,
			     mem_get_byte(cpu_get_HL()) == 255 ? TRUE : FALSE);
		break;

	case 0x36: // LD (HL),d8
		length = 2;
		duration = 12;
		LD_mem_u8(cpu_get_HL(), u8);
		break;

	case 0x37: // SCF
		length = 1;
		duration = 4;
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
		cpu_set_flag(FLAG_CARRY, TRUE);
		break;

	case 0x38: // JR C,r8
		length = 2;

		if (!cpu_get_flag(FLAG_CARRY)) {
			duration = 8;
		} else {
			duration = 12;
			int8_t i8 = (int8_t)mem_get_byte(regs.PC + 1);
			regs.PC = (uint16_t)(
				(int16_t)regs.PC +
				(int16_t)i8); // TODO: check if final PC value is right
			add_lg = 0;
			regs.PC += 2;
		}
		break;

	case 0x39: // ADD HL,SP //TODO
		length = 1;
		duration = 8;
		cpu_set_flag(FLAG_SUB, FALSE);
		// TODO: check if HC flag is managed correctly
		cpu_set_flag(FLAG_HALF_CARRY,
			     regs.L + (regs.SP & 0x00FF) > 0xFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY,
			     (uint32_t)cpu_get_HL() + (uint32_t)cpu_get_SP() >
					     0xFFFF ?
				     TRUE :
				     FALSE);
		cpu_set_HL(cpu_get_HL() + cpu_get_SP());
		break;

	case 0x3A: // LD A,(HL-)
		length = 1;
		duration = 8;
		LD_reg_u8(&regs.A, mem_get_byte(cpu_get_HL()));
		cpu_set_HL(cpu_get_HL() - 1);
		break;

	case 0x3B: // DEC SP
		length = 1;
		duration = 8;
		cpu_set_SP(cpu_get_SP() - 1);
		break;

	case 0x3C: // INC A
		length = 1;
		duration = 4;
		INC_u8(&regs.A);
		break;

	case 0x3D: // DEC A
		length = 1;
		duration = 4;
		DEC_u8(&regs.A);
		break;

	case 0x3E: // LD A,d8
		length = 2;
		duration = 8;
		LD_reg_u8(&regs.A, mem_get_byte(regs.PC + 1));
		break;

	case 0x3F: // CCF
		length = 1;
		duration = 4;
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
		cpu_set_flag(FLAG_CARRY,
			     cpu_get_flag(FLAG_CARRY) ? FALSE : TRUE);
		break;

	// 0x4X ////////////////////////////////////////////////////////////////
	case 0x40: // LD B,B
		length = 1;
		duration = 4;
		regs.B = regs.B;
		break;

	case 0x41: // LD B,C
		length = 1;
		duration = 4;
		regs.B = regs.C;
		break;

	case 0x42: // LD B,D
		length = 1;
		duration = 4;
		regs.B = regs.D;
		break;

	case 0x43: // LD B,E
		length = 1;
		duration = 4;
		regs.B = regs.E;
		break;

	case 0x44: // LD B,H
		length = 1;
		duration = 4;
		regs.B = regs.H;
		break;

	case 0x45: // LD B,L
		length = 1;
		duration = 4;
		regs.B = regs.L;
		break;

	case 0x46: // LD B,(HL)
		length = 1;
		duration = 8;
		regs.B = mem_get_byte(cpu_get_HL());
		break;

	case 0x47: // LD B,A
		length = 1;
		duration = 4;
		regs.B = regs.A;
		break;

	case 0x48: // LD C,B
		length = 1;
		duration = 4;
		regs.C = regs.B;
		break;

	case 0x49: // LD C,C
		length = 1;
		duration = 4;
		regs.C = regs.C;
		break;

	case 0x4A: // LD C,D
		length = 1;
		duration = 4;
		regs.C = regs.D;
		break;

	case 0x4B: // LD C,E
		length = 1;
		duration = 4;
		regs.C = regs.E;
		break;

	case 0x4C: // LD C,H
		length = 1;
		duration = 4;
		regs.C = regs.H;
		break;

	case 0x4D: // LD C,L
		length = 1;
		duration = 4;
		regs.C = regs.L;
		break;

	case 0x4E: // LD C,(HL)
		length = 1;
		duration = 8;
		regs.C = mem_get_byte(cpu_get_HL());
		break;

	case 0x4F: // LD C,A
		length = 1;
		duration = 4;
		regs.C = regs.A;
		break;

	// 0x5X ////////////////////////////////////////////////////////////////
	case 0x50: // LD D,B
		length = 1;
		duration = 4;
		regs.D = regs.B;
		break;

	case 0x51: // LD D,C
		length = 1;
		duration = 4;
		regs.D = regs.C;
		break;

	case 0x52: // LD BDD
		length = 1;
		duration = 4;
		regs.D = regs.D;
		break;

	case 0x53: // LD D,E
		length = 1;
		duration = 4;
		regs.D = regs.E;
		break;

	case 0x54: // LD D,H
		length = 1;
		duration = 4;
		regs.D = regs.H;
		break;

	case 0x55: // LD D,L
		length = 1;
		duration = 4;
		regs.D = regs.L;
		break;

	case 0x56: // LD D,(HL)
		length = 1;
		duration = 8;
		regs.D = mem_get_byte(cpu_get_HL());
		break;

	case 0x57: // LD D,A
		length = 1;
		duration = 4;
		regs.D = regs.A;
		break;

	case 0x58: // LD E,B
		length = 1;
		duration = 4;
		regs.E = regs.B;
		break;

	case 0x59: // LD E,C
		length = 1;
		duration = 4;
		regs.E = regs.C;
		break;

	case 0x5A: // LD E,D
		length = 1;
		duration = 4;
		regs.E = regs.D;
		break;

	case 0x5B: // LD E,E
		length = 1;
		duration = 4;
		regs.E = regs.E;
		break;

	case 0x5C: // LD E,H
		length = 1;
		duration = 4;
		regs.E = regs.H;
		break;

	case 0x5D: // LD E,L
		length = 1;
		duration = 4;
		regs.E = regs.L;
		break;

	case 0x5E: // LD E,(HL)
		length = 1;
		duration = 8;
		regs.E = mem_get_byte(cpu_get_HL());
		break;

	case 0x5F: // LD E,A
		length = 1;
		duration = 4;
		regs.E = regs.A;
		break;

	// 0x6X ////////////////////////////////////////////////////////////////
	case 0x60: // LD H,B
		length = 1;
		duration = 4;
		regs.H = regs.B;
		break;

	case 0x61: // LD H,C
		length = 1;
		duration = 4;
		regs.H = regs.C;
		break;

	case 0x62: // LD H,D
		length = 1;
		duration = 4;
		regs.H = regs.D;
		break;

	case 0x63: // LD H,E
		length = 1;
		duration = 4;
		regs.H = regs.E;
		break;

	case 0x64: // LD H,H
		length = 1;
		duration = 4;
		regs.H = regs.H;
		break;

	case 0x65: // LD H,L
		length = 1;
		duration = 4;
		regs.H = regs.L;
		break;

	case 0x66: // LD D,(HL)
		length = 1;
		duration = 8;
		regs.H = mem_get_byte(cpu_get_HL());
		break;

	case 0x67: // LD H,A
		length = 1;
		duration = 4;
		regs.H = regs.A;
		break;

	case 0x68: // LD L,B
		length = 1;
		duration = 4;
		regs.L = regs.B;
		break;

	case 0x69: // LD L,C
		length = 1;
		duration = 4;
		regs.L = regs.C;
		break;

	case 0x6A: // LD L,D
		length = 1;
		duration = 4;
		regs.L = regs.D;
		break;

	case 0x6B: // LD L,E
		length = 1;
		duration = 4;
		regs.L = regs.E;
		break;

	case 0x6C: // LD L,H
		length = 1;
		duration = 4;
		regs.L = regs.H;
		break;

	case 0x6D: // LD L,L
		length = 1;
		duration = 4;
		regs.L = regs.L;
		break;

	case 0x6E: // LD L,(HL)
		length = 1;
		duration = 8;
		regs.L = mem_get_byte(cpu_get_HL());
		break;

	case 0x6F: // LD L,A
		length = 1;
		duration = 4;
		regs.L = regs.A;
		break;

	// 0x7X ////////////////////////////////////////////////////////////////
	case 0x70: // LD (HL),B
		length = 1;
		duration = 8;
		mem_set_byte(cpu_get_HL(), regs.B);
		break;

	case 0x71: // LD (HL),C
		length = 1;
		duration = 8;
		mem_set_byte(cpu_get_HL(), regs.C);
		break;

	case 0x72: // LD (HL),D
		length = 1;
		duration = 8;
		mem_set_byte(cpu_get_HL(), regs.D);
		break;

	case 0x73: // LD (HL),E
		length = 1;
		duration = 8;
		mem_set_byte(cpu_get_HL(), regs.E);
		break;

	case 0x74: // LD (HL),H
		length = 1;
		duration = 8;
		mem_set_byte(cpu_get_HL(), regs.H);
		break;

	case 0x75: // LD (HL),L
		length = 1;
		duration = 8;
		mem_set_byte(cpu_get_HL(), regs.L);
		break;

	case 0x76: // HALT
		length = 1;
		duration = 4;
		// TODO: HALT behavior ?
		break;

	case 0x77: // LD (HL),A
		length = 1;
		duration = 8;
		mem_set_byte(cpu_get_HL(), regs.A);
		break;

	case 0x78: // LD A,B
		length = 1;
		duration = 4;
		regs.A = regs.B;
		break;

	case 0x79: // LD A,C
		length = 1;
		duration = 4;
		regs.A = regs.C;
		break;

	case 0x7A: // LD A,D
		length = 1;
		duration = 4;
		regs.A = regs.D;
		break;

	case 0x7B: // LD A,E
		length = 1;
		duration = 4;
		regs.A = regs.E;
		break;

	case 0x7C: // LD A,H
		length = 1;
		duration = 4;
		regs.A = regs.H;
		break;

	case 0x7D: // LD A,L
		length = 1;
		duration = 4;
		regs.A = regs.L;
		break;

	case 0x7E: // LD A,(HL)
		length = 1;
		duration = 4;
		regs.A = mem_get_byte(cpu_get_HL());
		break;

	case 0x7F: // LD A,A
		length = 1;
		duration = 4;
		regs.A = regs.A;
		break;

	// 0x8X ////////////////////////////////////////////////////////////////
	case 0x80: // ADD A,B
		length = 1;
		duration = 4;
		ADD_to_A(regs.B);
		break;

	case 0x81: // ADD A,C
		length = 1;
		duration = 4;
		ADD_to_A(regs.C);
		break;

	case 0x82: // ADD A,D
		length = 1;
		duration = 4;
		ADD_to_A(regs.D);
		break;

	case 0x83: // ADD A,E
		length = 1;
		duration = 4;
		ADD_to_A(regs.E);
		break;

	case 0x84: // ADD A,H
		length = 1;
		duration = 4;
		ADD_to_A(regs.H);
		break;

	case 0x85: // ADD A,L
		length = 1;
		duration = 4;
		ADD_to_A(regs.L);
		break;

	case 0x86: // ADD A,(HL)
		length = 1;
		duration = 8;
		ADD_to_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0x87: // ADD A,A
		length = 1;
		duration = 4;
		ADD_to_A(regs.A);
		break;

	case 0x88: // ADC A,B
		length = 1;
		duration = 4;
		ADC_to_A(regs.B);
		break;

	case 0x89: // ADC A,C
		length = 1;
		duration = 4;
		ADC_to_A(regs.C);
		break;

	case 0x8A: // ADC A,D
		length = 1;
		duration = 4;
		ADC_to_A(regs.D);
		break;

	case 0x8B: // ADC A,E
		length = 1;
		duration = 4;
		ADC_to_A(regs.E);
		break;

	case 0x8C: // ADC A,H
		length = 1;
		duration = 4;
		ADC_to_A(regs.H);
		break;

	case 0x8D: // ADC A,L
		length = 1;
		duration = 4;
		ADC_to_A(regs.L);
		break;

	case 0x8E: // ADC A,(HL)
		length = 1;
		duration = 8;
		ADC_to_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0x8F: // ADC A,A
		length = 1;
		duration = 4;
		ADC_to_A(regs.A);
		break;

	// 0x9X ////////////////////////////////////////////////////////////////
	case 0x90: // SUB A,B
		length = 1;
		duration = 4;
		SUB_to_A(regs.B);
		break;

	case 0x91: // SUB A,C
		length = 1;
		duration = 4;
		SUB_to_A(regs.C);
		break;

	case 0x92: // SUB A,D
		length = 1;
		duration = 4;
		SUB_to_A(regs.D);
		break;

	case 0x93: // SUB A,E
		length = 1;
		duration = 4;
		SUB_to_A(regs.E);
		break;

	case 0x94: // SUB A,H
		length = 1;
		duration = 4;
		SUB_to_A(regs.H);
		break;

	case 0x95: // SUB A,L
		length = 1;
		duration = 4;
		SUB_to_A(regs.L);
		break;

	case 0x96: // SUB A,(HL)
		length = 1;
		duration = 8;
		SUB_to_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0x97: // SUB A,A
		length = 1;
		duration = 4;
		SUB_to_A(regs.A);
		break;

	case 0x98: // SBC A,B
		length = 1;
		duration = 4;
		SBC_to_A(regs.B);
		break;

	case 0x99: // SBC A,C
		length = 1;
		duration = 4;
		SBC_to_A(regs.C);
		break;

	case 0x9A: // SBC A,D
		length = 1;
		duration = 4;
		SBC_to_A(regs.D);
		break;

	case 0x9B: // SBC A,E
		length = 1;
		duration = 4;
		SBC_to_A(regs.E);
		break;

	case 0x9C: // SBC A,H
		length = 1;
		duration = 4;
		SBC_to_A(regs.H);
		break;

	case 0x9D: // SBC A,L
		length = 1;
		duration = 4;
		SBC_to_A(regs.L);
		break;

	case 0x9E: // SBC A,(HL)
		length = 1;
		duration = 8;
		SBC_to_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0x9F: // SBC A,A
		length = 1;
		duration = 4;
		SBC_to_A(regs.A);
		break;

	// 0xAX ////////////////////////////////////////////////////////////////
	case 0xA0: // AND A,B
		length = 1;
		duration = 4;
		AND_with_A(regs.B);
		break;

	case 0xA1: // AND A,C
		length = 1;
		duration = 4;
		AND_with_A(regs.C);
		break;

	case 0xA2: // AND A,D
		length = 1;
		duration = 4;
		AND_with_A(regs.D);
		break;

	case 0xA3: // AND A,E
		length = 1;
		duration = 4;
		AND_with_A(regs.E);
		break;

	case 0xA4: // AND A,H
		length = 1;
		duration = 4;
		AND_with_A(regs.H);
		break;

	case 0xA5: // AND A,L
		length = 1;
		duration = 4;
		AND_with_A(regs.L);
		break;

	case 0xA6: // AND A,(HL)
		length = 1;
		duration = 8;
		AND_with_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0xA7: // AND A,A
		length = 1;
		duration = 4;
		AND_with_A(regs.A);
		break;

	case 0xA8: // XOR A,B
		length = 1;
		duration = 4;
		XOR_with_A(regs.B);
		break;

	case 0xA9: // XOR A,C
		length = 1;
		duration = 4;
		XOR_with_A(regs.C);
		break;

	case 0xAA: // XOR A,D
		length = 1;
		duration = 4;
		XOR_with_A(regs.D);
		break;

	case 0xAB: // XOR A,E
		length = 1;
		duration = 4;
		XOR_with_A(regs.E);
		break;

	case 0xAC: // XOR A,H
		length = 1;
		duration = 4;
		XOR_with_A(regs.H);
		break;

	case 0xAD: // XOR A,L
		length = 1;
		duration = 4;
		XOR_with_A(regs.L);
		break;

	case 0xAE: // XOR A,(HL)
		length = 1;
		duration = 8;
		XOR_with_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0xAF: // XOR A,A
		length = 1;
		duration = 4;
		XOR_with_A(regs.A);
		break;

	// 0xBX ////////////////////////////////////////////////////////////////
	case 0xB0: // OR A,B
		length = 1;
		duration = 4;
		OR_with_A(regs.B);
		break;

	case 0xB1: // OR A,C
		length = 1;
		duration = 4;
		OR_with_A(regs.C);
		break;

	case 0xB2: // OR A,D
		length = 1;
		duration = 4;
		OR_with_A(regs.D);
		break;

	case 0xB3: // OR A,E
		length = 1;
		duration = 4;
		OR_with_A(regs.E);
		break;

	case 0xB4: // OR A,H
		length = 1;
		duration = 4;
		OR_with_A(regs.H);
		break;

	case 0xB5: // OR A,L
		length = 1;
		duration = 4;
		OR_with_A(regs.L);
		break;

	case 0xB6: // OR A,(HL)
		length = 1;
		duration = 8;
		OR_with_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0xB7: // OR A,A
		length = 1;
		duration = 4;
		OR_with_A(regs.A);
		break;

	case 0xB8: // XOR A,B
		length = 1;
		duration = 4;
		CP_with_A(regs.B);
		break;

	case 0xB9: // CP A,C
		length = 1;
		duration = 4;
		CP_with_A(regs.C);
		break;

	case 0xBA: // CP A,D
		length = 1;
		duration = 4;
		CP_with_A(regs.D);
		break;

	case 0xBB: // CP A,E
		length = 1;
		duration = 4;
		CP_with_A(regs.E);
		break;

	case 0xBC: // CP A,H
		length = 1;
		duration = 4;
		CP_with_A(regs.H);
		break;

	case 0xBD: // CP A,L
		length = 1;
		duration = 4;
		CP_with_A(regs.L);
		break;

	case 0xBE: // CP A,(HL)
		length = 1;
		duration = 8;
		CP_with_A(mem_get_byte(cpu_get_HL()));
		break;

	case 0xBF: // CP A,A
		length = 1;
		duration = 4;
		CP_with_A(regs.A);
		break;

	// 0xCX ////////////////////////////////////////////////////////////////
	case 0xC0: // RET NZ
		length = 1;
		if(cpu_get_flag(FLAG_ZERO) == FALSE) {
			duration = 20;
			// TODO: not sure is POP is needed
			cpu_set_PC(SP_pop());
			add_lg = 0;
		}
		else {
			duration = 8;
		}
		break;

	case 0xC1: // POP BC
		length = 1;
		duration = 12;
		cpu_set_BC(SP_pop());
		break;

	case 0xC2: // JP NZ,a16
		length = 3;
		if(cpu_get_flag(FLAG_ZERO) == FALSE) {
			duration = 16;
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xC3: // JP a16
		length = 3;
		duration = 16;
		cpu_set_PC(u16);
		add_lg = 0;
		break;

	case 0xC4: // CALL NZ,a16
		length = 3;
		if(cpu_get_flag(FLAG_ZERO) == FALSE) {
			duration = 24;
			SP_push(cpu_get_PC());
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xC5: // PUSH BC
		length = 1;
		duration = 16;
		SP_push(cpu_get_BC());
		break;

	case 0xC6: // ADD A,d8
		length = 2;
		duration = 8;
		ADD_to_A(mem_get_byte(regs.PC + 1));
		break;

	case 0xC7: // RST 00H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0000);
		break;

	case 0xC8: // RET Z
		length = 1;
		if(cpu_get_flag(FLAG_ZERO) == TRUE) {
			duration = 20;
			// TODO: not sure is POP is needed
			cpu_set_PC(SP_pop());
			add_lg = 0;
		}
		else {
			duration = 8;
		}
		break;

	case 0xC9: // RET
		length = 1;
		duration = 16;
		// TODO: not sure is POP is needed
		cpu_set_PC(SP_pop());
		add_lg = 0;
		break;

	case 0xCA: // JP Z,a16
		length = 3;
		if(cpu_get_flag(FLAG_ZERO) == TRUE) {
			duration = 16;
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xCB: // PREFIX CB => TODO
		length = 2;
		duration = 8;	// TODO: adjust for some CB instruction which are 16
		cpu_exec_opcode_CB(mem_get_byte(regs.PC + 1));
		break;

	case 0xCC: // CALL Z,a16
		length = 3;
		if(cpu_get_flag(FLAG_ZERO) == TRUE) {
			duration = 24;
			SP_push(cpu_get_PC());
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xCD: // CALL a16
		length = 3;
		duration = 24;
		SP_push(cpu_get_PC() + 3);
		cpu_set_PC(u16);
		add_lg = 0;
		break;

	case 0xCE: // ADC A,d8
		length = 2;
		duration = 8;
		ADC_to_A(mem_get_byte(regs.PC + 1));
		break;

	case 0xCF: // RST 08H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0008);
		add_lg = 0;
		break;

	// 0xDX ////////////////////////////////////////////////////////////////
	case 0xD0: // RET NC
		length = 1;
		if(cpu_get_flag(FLAG_CARRY) == FALSE) {
			duration = 20;
			// TODO: not sure is POP is needed
			cpu_set_PC(SP_pop());
			add_lg = 0;
		}
		else {
			duration = 8;
		}
		break;

	case 0xD1: // POP DE
		length = 1;
		duration = 12;
		cpu_set_DE(SP_pop());
		break;

	case 0xD2: // JP NC,a16
		length = 3;
		if(cpu_get_flag(FLAG_CARRY) == FALSE) {
			duration = 16;
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xD4: // CALL NC,a16
		length = 3;
		if(cpu_get_flag(FLAG_CARRY) == FALSE) {
			duration = 24;
			SP_push(cpu_get_PC());
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xD5: // PUSH DE
		length = 1;
		duration = 16;
		SP_push(cpu_get_DE());
		break;

	case 0xD6: // SUB A,d8
		length = 2;
		duration = 8;
		SUB_to_A(u8);
		break;

	case 0xD7: // RST 10H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0010);
		add_lg = 0;
		break;

	case 0xD8: // RET C
		length = 1;
		if(cpu_get_flag(FLAG_CARRY) == TRUE) {
			duration = 20;
			// TODO: not sure is POP is needed
			cpu_set_PC(SP_pop());
			add_lg = 0;
		}
		else {
			duration = 8;
		}
		break;

	case 0xD9: // RETI
		length = 1;
		duration = 16;
		// TODO: not sure is POP is needed
		cpu_set_PC(SP_pop());
		cpu_interrupts_enabled = 1;
		add_lg = 0;
		break;

	case 0xDA: // JP C,a16
		length = 3;
		if(cpu_get_flag(FLAG_CARRY) == TRUE) {
			duration = 16;
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xDC: // CALL C,a16
		length = 3;
		if(cpu_get_flag(FLAG_CARRY) == TRUE) {
			duration = 24;
			SP_push(cpu_get_PC());
			cpu_set_PC(u16);
			add_lg = 0;
		}
		else {
			duration = 12;
		}
		break;

	case 0xDE: // SDC A,d8
		length = 2;
		duration = 8;
		SBC_to_A(mem_get_byte(regs.PC + 1));
		break;

	case 0xDF: // RST 18H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0018);
		add_lg = 0;
		break;

	// 0xEX ////////////////////////////////////////////////////////////////
	case 0xE0: // LDH (a8),A
		length = 2;
		duration = 12;
		mem_set_byte(0xFF00 + u8, regs.A);
		break;

	case 0xE1: // POP HL
		length = 1;
		duration = 12;
		cpu_set_HL(SP_pop());
		break;

	case 0xE2: // LD (C),A
		length = 1;
		duration = 8;
		LD_mem_u8(0xFF00 + cpu_get_C(), regs.A);
		break;

	case 0xE5: // PUSH HL
		length = 1;
		duration = 16;
		SP_push(cpu_get_HL());
		break;

	case 0xE6: // AND d8
		length = 2;
		duration = 8;
		AND_with_A(u8);
		break;

	case 0xE7: // RST 20H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0020);
		add_lg = 0;
		break;

	case 0xE8: // ADD SP,r8
		length = 2;
		duration = 16;

		r8 = (int8_t) u8;

		cpu_set_SP((uint16_t)(cpu_get_SP() + (int16_t)r8));

		cpu_set_flag(FLAG_ZERO, FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);

		// TODO: test carry corncases
		cpu_set_flag(FLAG_HALF_CARRY,
		     (regs.SP & 0x0FFF) + r8 > 0x0FFF ? TRUE : FALSE);

		cpu_set_flag(FLAG_CARRY,
		     (regs.SP & 0xFFFF) + r8 > 0xFFFF ? TRUE : FALSE);

		break;

	case 0xE9: // JP (HL)
		length = 1;
		duration = 4;
		cpu_set_PC(cpu_get_HL());
		add_lg = 0;
		break;

	case 0xEA: // LD (a16),A
		length = 3;
		duration = 16;
		LD_mem_u8(u16, regs.A);
		break;

	case 0xEE: // XOR d8
		length = 2;
		duration = 8;
		XOR_with_A(u16);
		break;

	case 0xEF: // RST 28H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0028);
		add_lg = 0;
		break;

	// 0xFX ////////////////////////////////////////////////////////////////
	case 0xF0: // LDH A,(a8)
		length = 2;
		duration = 12;
		regs.A = mem_get_byte(0xFF00 | u8);
		break;

	case 0xF1: // POP AF
		length = 1;
		duration = 12;
		cpu_set_AF(SP_pop());
		/*cpu_set_flag(FLAG_ZERO, TRUE);
		cpu_set_flag(FLAG_SUB, TRUE);
		cpu_set_flag(FLAG_HALF_CARRY, TRUE);
		cpu_set_flag(FLAG_CARRY, TRUE);*/
		break;

	case 0xF2: // LD (C),A
		length = 2;
		duration = 8;
		regs.A = mem_get_byte(0xFF00 + cpu_get_C());
		break;

	case 0xF3: // DI
		length = 1;
		duration = 4;
		cpu_interrupts_enabled = 0;
		break;

	case 0xF5: // PUSH AF
		length = 1;
		duration = 16;
		SP_push(cpu_get_AF());
		break;

	case 0xF6: // OR d8
		length = 2;
		duration = 8;
		OR_with_A(u8);
		break;

	case 0xF7: // RST 30H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0030);
		add_lg = 0;
		break;

	case 0xF8: // LD HL,SP+r8
		length = 2;
		duration = 12;
		r8 = (int8_t) u8;
		cpu_set_flag(FLAG_ZERO, FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		
		// TODO: test carry corner cases
		cpu_set_flag(FLAG_HALF_CARRY,
		     (regs.SP & 0x0FFF) + r8 > 0x0FFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY,
		     (regs.SP & 0xFFFF) + r8 > 0xFFFF ? TRUE : FALSE);
		
		cpu_set_HL(cpu_get_SP() + (int8_t)u8);
		break;

	case 0xF9: // LD SP,HL
		length = 1;
		duration = 8;
		cpu_set_SP(cpu_get_HL());
		break;

	case 0xFA: // LD A,(a16)
		length = 3;
		duration = 16;
		regs.A = mem_get_byte(u16);
		break;

	case 0xFB: // EI
		length = 1;
		duration = 4;
		cpu_interrupts_enabled = 1;
		break;

	case 0xFE: // CP d8
		length = 2;
		duration = 8;
		CP_with_A(u8);
		break;

	case 0xFF: // RST 38H
		length = 1;
		duration = 16;
		SP_push(cpu_get_PC()+1);
		cpu_set_PC(0x0038);
		add_lg = 0;
		break;



	default:
		printf("[ERROR][%s:%d] unkown opcode 0x%x!\n", __func__,
		       __LINE__, opcode);
		exit(0);
	}



	// TOOD: wait cycles here ?

	if (length == 0 || duration == 0)
		printf("[ERROR][%s:%d] invalid opcode length or duration\n",
		       __func__, __LINE__);

	*opcode_length = length;
	*opcode_duration = duration;

	if(add_lg)
        cpu_set_PC(cpu_get_PC() + length);

	return 0;
}

static uint8_t cpu_exec_opcode_CB(uint8_t opcode)
{
	uint8_t u8;

	switch (opcode) {
	// 0x0X ////////////////////////////////////////////////////////////////
	case 0x00: // RLC B
		RLC(&regs.B);
		break;

	case 0x01: // RLC C
		RLC(&regs.C);
		break;

	case 0x02: // RLC D
		RLC(&regs.D);
		break;

	case 0x03: // RLC E
		RLC(&regs.E);
		break;

	case 0x04: // RLC H
		RLC(&regs.H);
		break;

	case 0x05: // RLC L
		RLC(&regs.L);
		break;

	case 0x06: // RLC (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		RLC(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x07: // RLC A
		RLC(&regs.A);
		break;

	case 0x08: // RRC B
		RRC(&regs.B);
		break;

	case 0x09: // RRC C
		RRC(&regs.C);
		break;

	case 0x0A: // RRC D
		RRC(&regs.D);
		break;

	case 0x0B: // RRC E
		RRC(&regs.E);
		break;

	case 0x0C: // RRC H
		RRC(&regs.H);
		break;

	case 0x0D: // RRC L
		RRC(&regs.L);
		break;

	case 0x0E: // RRC (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		RRC(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x0F: // RRC A
		RRC(&regs.A);
		break;		

	// 0x1X ////////////////////////////////////////////////////////////////
	case 0x10: // RL B
		RL(&regs.B);
		break;

	case 0x11: // RL C
		RL(&regs.C);
		break;

	case 0x12: // RL D
		RL(&regs.D);
		break;

	case 0x13: // RL E
		RL(&regs.E);
		break;

	case 0x14: // RL H
		RL(&regs.H);
		break;

	case 0x15: // RL L
		RL(&regs.L);
		break;

	case 0x16: // RL (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		RL(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x17: // RL A
		RL(&regs.A);
		break;

	case 0x18: // RR B
		RR(&regs.B);
		break;

	case 0x19: // RR C
		RR(&regs.C);
		break;

	case 0x1A: // RR D
		RR(&regs.D);
		break;

	case 0x1B: // RR E
		RR(&regs.E);
		break;

	case 0x1C: // RR H
		RR(&regs.H);
		break;

	case 0x1D: // RR L
		RR(&regs.L);
		break;

	case 0x1E: // RR (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		RR(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x1F: // RR A
		RR(&regs.A);
		break;	

	// 0x2X ////////////////////////////////////////////////////////////////
	case 0x20: // SLA B
		SLA(&regs.B);
		break;

	case 0x21: // SLA C
		SLA(&regs.C);
		break;

	case 0x22: // SLA D
		SLA(&regs.D);
		break;

	case 0x23: // SLA E
		SLA(&regs.E);
		break;

	case 0x24: // SLA H
		SLA(&regs.H);
		break;

	case 0x25: // SLA L
		SLA(&regs.L);
		break;

	case 0x26: // SLA (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		SLA(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x27: // SLA A
		SLA(&regs.A);
		break;

	case 0x28: // SRA B
		SRA(&regs.B);
		break;

	case 0x29: // SRA C
		SRA(&regs.C);
		break;

	case 0x2A: // SRA D
		SRA(&regs.D);
		break;

	case 0x2B: // SRA E
		SRA(&regs.E);
		break;

	case 0x2C: // SRA H
		SRA(&regs.H);
		break;

	case 0x2D: // SRA L
		SRA(&regs.L);
		break;

	case 0x2E: // SRA (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		SRA(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x2F: // SRA A
		SRA(&regs.A);
		break;	

	// 0x3X ////////////////////////////////////////////////////////////////
	case 0x30: // SWAP B
		SWAP(&regs.B);
		break;

	case 0x31: // SWAP C
		SWAP(&regs.C);
		break;

	case 0x32: // SWAP D
		SWAP(&regs.D);
		break;

	case 0x33: // SWAP E
		SWAP(&regs.E);
		break;

	case 0x34: // SWAP H
		SWAP(&regs.H);
		break;

	case 0x35: // SWAP L
		SWAP(&regs.L);
		break;

	case 0x36: // SWAP (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		SWAP(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x37: // SWAP A
		SWAP(&regs.A);
		break;

	case 0x38: // SRL B
		SRL(&regs.B);
		break;

	case 0x39: // SRL C
		SRL(&regs.C);
		break;

	case 0x3A: // SRL D
		SRL(&regs.D);
		break;

	case 0x3B: // SRL E
		SRL(&regs.E);
		break;

	case 0x3C: // SRL H
		SRL(&regs.H);
		break;

	case 0x3D: // SRL L
		SRL(&regs.L);
		break;

	case 0x3E: // SRL (HL)
		u8 = mem_get_byte(cpu_get_HL()); 	
		SRL(&u8);
		mem_set_byte(cpu_get_HL(), u8);
		break;

	case 0x3F: // SRL A
		SRL(&regs.A);
		break;	

	default:
		{
		uint8_t reg2test = (opcode - 0x40) % 8;
		//printf("[opcode=0x%x] bit2test = %d, reg2test = %d\n", opcode, bit2test, reg2test);
		uint8_t *p_reg = NULL;
		uint16_t opcode_on_addr = 0;
		switch (reg2test) {
			case 0:
				p_reg = &regs.B;
				break;
			case 1:
				p_reg = &regs.C;
				break;
			case 2:
				p_reg = &regs.D;
				break;
			case 3:
				p_reg = &regs.E;
				break;
			case 4:
				p_reg = &regs.H;
				break;
			case 5:
				p_reg = &regs.L;
				break;
			case 6:
				opcode_on_addr = 1;
				break;
			case 7:
				p_reg = &regs.A;
				break;
		}
		if (opcode >= 0x40 && opcode <= 0x7F) {
			uint8_t bit2test = (opcode - 0x40) / 8;

			if(opcode_on_addr) {
				u8 = mem_get_byte(cpu_get_HL()); 	
				BIT(bit2test, &u8);
			} else {
				BIT(bit2test, p_reg);
			}
		}
		else if (opcode >= 0x80 && opcode <= 0xBF) {
			uint8_t bit2test = (opcode - 0x80) / 8;

			if(opcode_on_addr) {
				u8 = mem_get_byte(cpu_get_HL()); 	
				RES(bit2test, &u8);
				mem_set_byte(cpu_get_HL(), u8);
			} else {
				RES(bit2test, p_reg);
			}
		}
		else if (opcode >= 0xC0 && opcode <= 0xFF) {
			uint8_t bit2test = (opcode - 0xC0) / 8;

			if(opcode_on_addr) {
				u8 = mem_get_byte(cpu_get_HL()); 	
				SET(bit2test, &u8);
				mem_set_byte(cpu_get_HL(), u8);
			} else {
				SET(bit2test, p_reg);
			}
		}
		else {
			printf("[ERROR][%s:%d] unkown opcode 0x%x!\n", __func__,
			    __LINE__, opcode);
		}
		break;
		}
	}
}

void cpu_init()
{
	cpu_set_AF(0x01);
	cpu_set_F(0xB0);
	cpu_set_BC(0x0013);
	cpu_set_DE(0x00D8);
	cpu_set_HL(0x014D);
	cpu_set_SP(0xFFFE);
	cpu_set_PC(0x0100);
}