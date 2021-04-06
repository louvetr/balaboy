
#include "cpu.h"

int set_cpu_flag(struct cpu_registers *regs, cpu_flag_name flag,
		 cpu_flag_value value)
{
	switch (flag) {
	case FLAG_ZERO:
		regs->F = regs->F & 0x7F | value << 7;
		break;
	case FLAG_SUB:
		regs->F = regs->F & 0xBF | value << 6;
		break;
	case FLAG_HALF_CARRY:
		regs->F = regs->F & 0xDF | value << 5;
		break;
	case FLAG_CARRY:
		regs->F = regs->F & 0xEF | value << 4;
		break;
	default:
		printf("[ERROR][%s:%d] invalid flag\n", __func__, __LINE__);
	}

	return 0;
}

uint8_t exec_opcode(uint8_t opcode, struct cpu_registers *regs, uint8_t *mem)
{
	uint8_t length = 0; // length in byte
	uint8_t duration = 0; // duration in clock cycles

	// go to byte after the opcode one
	//regs->PC++;

	uint8_t u8 = mem[regs->PC + 1];

	// Get the u16 value after opcode even if not needed.
	// Done before the switch per pure laziness (and to avoid possible error since we have 250+ case)

	// WARNING: check u8 LSB is OK for all u16 used by opcodes
	uint16_t u16 =
		mem[regs->PC + 2] << 8 | u8; // OP B1 B2 => B1 is LSB, B2 is MSB

	printf("regs->PC = %d, \n", regs->PC);
	printf("u8 = 0x%x, mem[%d] = 0x%x, mem[%d] = 0x%x =====>>>> u16 = 0x%04x" PRIu16
	       "\n",
	       u8, (regs->PC) + 1, mem[regs->PC + 1], (regs->PC) + 2,
	       mem[regs->PC + 2], u16);

	switch (opcode) {
	case 0x00: // NOP
		length = 1;
		duration = 4;
		break;

	case 0x01: // LD BC,d16
		length = 3;
		duration = 12;
		memcpy(&regs->B, &u16,
		       sizeof(uint16_t)); // TODO: check if LSB or MSB order
		//memcpy(&regs->BC, &u16, sizeof(uint16_t));   // TODO: check if LSB or MSB order
		break;

	case 0x02: // LD (BC),A
		length = 1;
		duration = 8;
		//memcpy(&regs->B, &((uint16_t)regs->A), sizeof(uint16_t));
		u16 = regs->A;
		memcpy(&regs->B, &u16, sizeof(uint16_t));
		break;

	case 0x03: // INC BC
		length = 1;
		duration = 8;
		(uint16_t)(regs->B)++;
		break;

	case 0x04: // INC B
		length = 1;
		duration = 4;
		regs->B++;
		set_cpu_flag(regs, FLAG_ZERO, regs->B == 0 ? TRUE : FALSE);
		set_cpu_flag(regs, FLAG_SUB, FALSE);
		set_cpu_flag(regs, FLAG_HALF_CARRY, regs->B == 0 ? TRUE : FALSE);
		break;

	case 0x05: // DEC B
		length = 1;
		duration = 4;
		regs->B--;
		set_cpu_flag(regs, FLAG_ZERO, regs->B == 0 ? TRUE : FALSE);
		set_cpu_flag(regs, FLAG_SUB, TRUE);
		set_cpu_flag(regs, FLAG_HALF_CARRY, regs->B == 255 ? TRUE : FALSE);
		break;

	case 0x06: // LD B,d8
		length = 2;
		duration = 8;
        regs->B = mem[regs->PC+1];
		break;

	case 0x07: // RLC A
		length = 1;
		duration = 4;
		set_cpu_flag(regs, FLAG_CARRY, regs->A & 0x80 ? TRUE : FALSE);
        regs->A = regs->A << 1;
		set_cpu_flag(regs, FLAG_ZERO, regs->A == 0 ? TRUE : FALSE);
		set_cpu_flag(regs, FLAG_SUB, FALSE);
		set_cpu_flag(regs, FLAG_HALF_CARRY, FALSE);
		break;

	default:
		printf("[ERROR][%s:%d] unkown opcode !\n", __func__, __LINE__);
	}

	// TOOD: wait cycles here ?

	if (length == 0 || duration == 0)
		printf("[ERROR][%s:%d] invalid opcode length or duration\n",
		       __func__, __LINE__);

	return length;
}
