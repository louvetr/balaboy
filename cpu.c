
#include "cpu.h"

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

uint8_t cpu_exec_opcode(uint8_t opcode, uint8_t *mem)
{
	uint8_t length = 0; // length in byte
	uint8_t duration = 0; // duration in clock cycles

	uint8_t u8 = mem[regs.PC + 1];

	// Get the u16 value after opcode even if not needed.
	// Done before the switch per pure laziness (and to avoid possible error since we have 250+ case)

	// TODO: WARNING: check u8 LSB is OK for all u16 used by opcodes
	uint16_t u16 =
		mem[regs.PC + 2] << 8 | u8; // OP B1 B2 => B1 is LSB, B2 is MSB

	uint32_t u32 = 0;

	switch (opcode) {
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
		u16 = regs.A;
		cpu_set_BC(u16);
		break;

	case 0x03: // INC BC
		length = 1;
		duration = 8;
		cpu_set_BC(cpu_get_BC()+1);
		break;

	case 0x04: // INC B
		length = 1;
		duration = 4;
		regs.B++;
		cpu_set_flag(FLAG_ZERO, regs.B == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, regs.B == 0 ? TRUE : FALSE);
		break;

	case 0x05: // DEC B
		length = 1;
		duration = 4;
		regs.B--;
		cpu_set_flag(FLAG_ZERO, regs.B == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, TRUE);
		cpu_set_flag(FLAG_HALF_CARRY, regs.B == 255 ? TRUE : FALSE);
		break;

	case 0x06: // LD B,d8
		length = 2;
		duration = 8;
        regs.B = mem[regs.PC+1];
		break;

	case 0x07: // RLC A
		length = 1;
		duration = 4;
		cpu_set_flag(FLAG_CARRY, regs.A & 0x80 ? TRUE : FALSE);
        regs.A = regs.A << 1;
		cpu_set_flag(FLAG_ZERO, regs.A == 0 ? TRUE : FALSE);
		cpu_set_flag(FLAG_SUB, FALSE);
		cpu_set_flag(FLAG_HALF_CARRY, FALSE);
		break;

	case 0x08: // LD (a16),SP
		length = 3;
		duration = 20;
		memcpy(&mem[u16], &regs.SP,
		       sizeof(uint16_t)); // TODO: check if LSB or MSB order
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
		regs.A = mem[cpu_get_BC()];
		break;

	case 0x0B: // DEC BC
		length = 1;
		duration = 8;
		cpu_set_BC(cpu_get_BC(regs) - 1);
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
