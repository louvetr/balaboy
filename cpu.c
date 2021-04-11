
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

/////////////////////////////////////////////////////////////////////////////////////
// public functions
/////////////////////////////////////////////////////////////////////////////////////

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

uint8_t cpu_exec_opcode(uint8_t opcode)
{
	uint8_t length = 0; // length in byte
	uint8_t duration = 0; // duration in clock cycles

	uint8_t u8 = mem_get_byte(regs.PC + 1);

	// Get the u16 value after opcode even if not needed.
	// Done before the switch per pure laziness (and to avoid possible error since we have 250+ case)

	// TODO: WARNING: check u8 LSB is OK for all u16 used by opcodes
	uint16_t u16 = mem_get_byte(regs.PC + 2) << 8 |
		       u8; // OP B1 B2 => B1 is LSB, B2 is MSB

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
		regs.A = (regs.A << 1) | cpu_get_flag(FLAG_CARRY);
		cpu_set_flag(FLAG_CARRY, regs.A & 0x80 ? TRUE : FALSE);
		cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
		break;

	case 0x18: // JR r8
		length = 2;
		duration = 12;
		int8_t i8 = (int8_t)mem_get_byte(regs.PC + 1);
		printf("[cpu.c] regs.PC + i8 = 0x%x = 0x%x + %d (0x%x)\n",
		       regs.PC + i8, regs.PC, i8, mem_get_byte(regs.PC + 1));
		printf("[cpu.c] int16_t (int16_t)regs.PC + (int16_t)i8 = %d + %d = %d\n",
		       (int16_t)regs.PC + (int16_t)i8, (int16_t)regs.PC,
		       (int16_t)i8);
		regs.PC = (uint16_t)(
			(int16_t)regs.PC +
			(int16_t)i8); // TODO: check if final PC value is right
		printf("[cpu.c] regs.PC = 0x%x\n", regs.PC);
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

		if (cpu_get_flag(FLAG_ZERO)) {
			duration = 8;
		} else {
			duration = 12;
			int8_t i8 = (int8_t)mem_get_byte(regs.PC + 1);
			regs.PC = (uint16_t)(
				(int16_t)regs.PC +
				(int16_t)i8); // TODO: check if final PC value is right
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
		}
		break;

	case 0x29: // AD HL,HL
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
		}
		break;

	case 0x39: // AD HL,SP
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
		duration = 4;
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
		duration = 4;
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
		duration = 4;
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
		duration = 4;
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
		duration = 4;
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
		duration = 4;
		regs.L = mem_get_byte(cpu_get_HL());
		break;

	case 0x6F: // LD L,A
		length = 1;
		duration = 4;
		regs.L = regs.A;
		break;

	default:
		printf("[ERROR][%s:%d] unkown opcode 0x%x!\n", __func__,
		       __LINE__, opcode);
	}

	// TOOD: wait cycles here ?

	if (length == 0 || duration == 0)
		printf("[ERROR][%s:%d] invalid opcode length or duration\n",
		       __func__, __LINE__);

	return length;
}
