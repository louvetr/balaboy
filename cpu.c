
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
    uint16_t SP;    // stack pointer
    uint16_t PC;    // program counter
};

// declare locally an instance of the register structure.
// This struct var has to be reach externally via getter and setter funcions
static struct cpu_registers regs;

/////////////////////////////////////////////////////////////////////////////////////
// private functions
/////////////////////////////////////////////////////////////////////////////////////

static void INC_u8(uint8_t *ptr) {
	(*ptr)++;
	cpu_set_flag(FLAG_ZERO, *ptr == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, FALSE);
	cpu_set_flag(FLAG_HALF_CARRY, *ptr == 0 ? TRUE : FALSE);
}

static void DEC_u8(uint8_t *ptr) {
	(*ptr)--;
	cpu_set_flag(FLAG_ZERO, *ptr == 0 ? TRUE : FALSE);
	cpu_set_flag(FLAG_SUB, TRUE);
	cpu_set_flag(FLAG_HALF_CARRY, *ptr == 255 ? TRUE : FALSE);
	// TODO: check HC flag
}

static void LD_mem_u8(uint16_t addr, uint8_t src) {
	mem_set_byte(addr, src);
}

static void LD_reg_u8(uint8_t *reg_u8, uint8_t src) {
	*reg_u8 = src;
}

static void LD_mem_u16(uint16_t addr, uint16_t src) {
	mem_set_byte(addr, src & 0x00FF);
	mem_set_byte(addr+1, src >> 8);
}

/////////////////////////////////////////////////////////////////////////////////////
// public functions
/////////////////////////////////////////////////////////////////////////////////////

void cpu_reset_registers(){
	memset(&regs, 0, sizeof(struct cpu_registers));
}

// getter of 8 bits registers
uint8_t cpu_get_A() { return regs.A; }
uint8_t cpu_get_B() { return regs.B; }
uint8_t cpu_get_C() { return regs.C; }
uint8_t cpu_get_D() { return regs.D; }
uint8_t cpu_get_E() { return regs.E; }
uint8_t cpu_get_F() { return regs.F; }
uint8_t cpu_get_H() { return regs.H; }
uint8_t cpu_get_L() { return regs.L; }

// setter of 8 bits registers
void cpu_set_A(uint8_t value) { regs.A = value; }
void cpu_set_B(uint8_t value) { regs.B = value; }
void cpu_set_C(uint8_t value) { regs.C = value; }
void cpu_set_D(uint8_t value) { regs.D = value; }
void cpu_set_E(uint8_t value) { regs.E = value; }
void cpu_set_F(uint8_t value) { regs.F = value; }
void cpu_set_H(uint8_t value) { regs.H = value; }
void cpu_set_L(uint8_t value) { regs.L = value; }

// getter of 16 bits registers
uint16_t cpu_get_AF(){
	return regs.A << 8 | regs.F;
}
uint16_t cpu_get_BC(){
	return regs.B << 8 | regs.C;
}
uint16_t cpu_get_DE(){
	return regs.D << 8 | regs.E;
}
uint16_t cpu_get_HL(){
	return regs.H << 8 | regs.L;
}

// getter of 16 bits registers
void cpu_set_AF(uint16_t value) {
	regs.A = (uint8_t)(value >> 8);
	regs.F = (uint8_t)(value & 0x00FF);
}
void cpu_set_BC(uint16_t value){
	regs.B = (uint8_t)(value >> 8);
	regs.C = (uint8_t)(value & 0x00FF);
}
void cpu_set_DE(uint16_t value){
	regs.D = (uint8_t)(value >> 8);
	regs.E = (uint8_t)(value & 0x00FF);
}
void cpu_set_HL(uint16_t value){
	regs.H = (uint8_t)(value >> 8);
	regs.L = (uint8_t)(value & 0x00FF);
}

// getter of 16 bits registers
uint16_t cpu_get_SP() { return regs.SP; }
uint16_t cpu_get_PC() { return regs.PC; }

// setter of 16 bits registers
void cpu_set_SP(uint16_t value) { regs.SP = value; }
void cpu_set_PC(uint16_t value) { regs.PC = value; } 


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

uint8_t cpu_exec_opcode(uint8_t opcode)
{
	uint8_t length = 0; // length in byte
	uint8_t duration = 0; // duration in clock cycles

	uint8_t u8 = mem_get_byte(regs.PC + 1);

	// Get the u16 value after opcode even if not needed.
	// Done before the switch per pure laziness (and to avoid possible error since we have 250+ case)

	// TODO: WARNING: check u8 LSB is OK for all u16 used by opcodes
	uint16_t u16 =
		mem_get_byte(regs.PC + 2) << 8 | u8; // OP B1 B2 => B1 is LSB, B2 is MSB

	uint32_t u32 = 0;

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
		cpu_set_BC(cpu_get_BC()+1);
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
		LD_reg_u8(&regs.B, mem_get_byte(regs.PC+1));
		break;

	case 0x07: // RLC A
		length = 1;
		duration = 4;
        regs.A = (regs.A << 1) | (regs.A >> 7);
		cpu_set_flag(FLAG_CARRY, regs.A & 0x80 ? TRUE : FALSE);
		cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
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
		cpu_set_flag(FLAG_HALF_CARRY, regs.L + regs.C > 0xFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY, (uint32_t)cpu_get_HL() + (uint32_t)cpu_get_BC() > 0xFFFF ? TRUE : FALSE);
		cpu_set_HL( cpu_get_HL() + cpu_get_BC());
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
		LD_reg_u8(&regs.C, mem_get_byte(regs.PC+1));
		break;

	case 0x0F: // RRC A
		length = 1;
		duration = 4;
		cpu_set_flag(FLAG_CARRY, regs.A & 0x01);
        regs.A = regs.A >> 1 | (regs.A & 0x01) << 7;
		// TODO: force ZERO flag to FALSE ?
		cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
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
		cpu_set_DE(cpu_get_DE()+1);
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
		LD_reg_u8(&regs.D, mem_get_byte(regs.PC+1));
		break;

	case 0x17: // RL A
		length = 1;
		duration = 4;
        regs.A = (regs.A << 1) | cpu_get_flag(FLAG_CARRY);
		cpu_set_flag(FLAG_CARRY, regs.A & 0x80 ? TRUE : FALSE);
		cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
		break;

	case 0x18: // JR r8
		length = 2;
		duration = 12;
		int8_t i8 = (int8_t)mem_get_byte(regs.PC+1);
		printf("[cpu.c] regs.PC + i8 = 0x%x = 0x%x + %d (0x%x)\n", regs.PC + i8, regs.PC, i8, mem_get_byte(regs.PC+1));
		printf("[cpu.c] int16_t (int16_t)regs.PC + (int16_t)i8 = %d + %d = %d\n",
		(int16_t)regs.PC + (int16_t)i8, (int16_t)regs.PC, (int16_t)i8);
		regs.PC = (uint16_t)((int16_t)regs.PC + (int16_t)i8); // TODO: check if final PC value is right
		printf("[cpu.c] regs.PC = 0x%x\n", regs.PC);
		break;	

	case 0x19: // ADD HL,DE
		length = 1;
		duration = 8;
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, regs.L + regs.E > 0xFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY, (uint32_t)cpu_get_HL() + (uint32_t)cpu_get_DE() > 0xFFFF ? TRUE : FALSE);
		cpu_set_HL( cpu_get_HL() + cpu_get_DE());
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
		LD_reg_u8(&regs.E, mem_get_byte(regs.PC+1));
		break;

	case 0x1F: // RR A
		length = 1;
		duration = 4;
		uint8_t tmp_carry = regs.A & 0x01;
        regs.A = regs.A >> 1 | cpu_get_flag(FLAG_CARRY) << 7;
		cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
		cpu_set_flag(FLAG_CARRY, tmp_carry);
		break;


	// 0x2X ////////////////////////////////////////////////////////////////
	case 0x20: // JR NZ,r8
		length = 2;

		if(cpu_get_flag(FLAG_ZERO)) {
			duration = 8;
		} else {
			duration = 12;
			int8_t i8 = (int8_t)mem_get_byte(regs.PC+1);
			regs.PC = (uint16_t)((int16_t)regs.PC + (int16_t)i8); // TODO: check if final PC value is right
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
		cpu_set_HL(cpu_get_HL()+1);
		break;

	case 0x23: // INC HL
		length = 1;
		duration = 8;
		cpu_set_HL(cpu_get_HL()+1);
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
		LD_reg_u8(&regs.H, mem_get_byte(regs.PC+1));
		break;

	case 0x27: // DAA
		length = 1;
		duration = 4;
		uint8_t D1 = regs.A >> 4;
		uint8_t D2 = regs.A & 0x0F;
		if (cpu_get_flag(FLAG_SUB))
		{
			if (cpu_get_flag(FLAG_SUB) | D2 > 9)
				D2 -= 6;
			if (cpu_get_flag(FLAG_CARRY))
				D1 -= 6;
			if (D1 > 9)
			{
				D1 -= 6;
				cpu_set_flag(FLAG_CARRY, TRUE);
			}
		}
		else
		{
			if (cpu_get_flag(FLAG_HALF_CARRY)) D2 += 6;
			if (cpu_get_flag(FLAG_CARRY)) D1 += 6;
			if (D2 > 9)
			{
				D2 -= 10;
				D1++;
			}
			if (D1 > 9)
			{
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

		if(!cpu_get_flag(FLAG_ZERO)) {
			duration = 8;
		} else {
			duration = 12;
			int8_t i8 = (int8_t)mem_get_byte(regs.PC+1);
			regs.PC = (uint16_t)((int16_t)regs.PC + (int16_t)i8); // TODO: check if final PC value is right
		}
		break;

	case 0x29: // AD HL,HL
		length = 1;
		duration = 8;
		cpu_set_flag(FLAG_SUB, FALSE);
		// TODO: check if HC flag is managed correctly
		cpu_set_flag(FLAG_HALF_CARRY, regs.L + regs.L > 0xFF ? TRUE : FALSE);
		cpu_set_flag(FLAG_CARRY, (uint32_t)cpu_get_HL() + (uint32_t)cpu_get_HL() > 0xFFFF ? TRUE : FALSE);
		cpu_set_HL( cpu_get_HL() + cpu_get_HL());
		break;

	case 0x2A: // LD A,(HL+)
		length = 1;
		duration = 8;
		LD_reg_u8(&regs.A, mem_get_byte(cpu_get_HL()));
		cpu_set_HL(cpu_get_HL()+1);
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
		LD_reg_u8(&regs.L, mem_get_byte(regs.PC+1));
		break;

	case 0x2F: // CP L
		length = 1;
		duration = 4;
		regs.A = ~regs.A;
		cpu_set_flag(FLAG_SUB, TRUE);
		cpu_set_flag(FLAG_HALF_CARRY, TRUE);
		break;

	default:
		printf("[ERROR][%s:%d] unkown opcode 0x%x!\n", __func__, __LINE__, opcode);
	}

	// TOOD: wait cycles here ?

	if (length == 0 || duration == 0)
		printf("[ERROR][%s:%d] invalid opcode length or duration\n",
		       __func__, __LINE__);

	return length;
}
