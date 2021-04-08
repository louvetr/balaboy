#include <errno.h>
#include <stdlib.h>
#include "cpu.h"

struct opcode_info {
	uint8_t code;
	const char *name;
	uint8_t byte1;
	uint8_t byte2;
};

struct opcode_info opcode_dict[12] = {
	{.code = 0x00, .name = "NOP", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x01, .name = "LD BC,d16", .byte1 = 0x21, .byte2 = 0x43 },
	{.code = 0x02, .name = "LD (BC),A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x03, .name = "INC BC", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x04, .name = "INC B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x05, .name = "DEC B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x06, .name = "LD B,d8", .byte1 = 0x42, .byte2 = 0x00 },
	{.code = 0x07, .name = "RLC A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x08, .name = "LD (a16),SP", .byte1 = 0x34, .byte2 = 0x12 },
	{.code = 0x09, .name = "ADD HL,BC", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x0A, .name = "LD A,(BC)", .byte1 = 0x00, .byte2 = 0x42 },
	{.code = 0x0B, .name = "DEC BC", .byte1 = 0x00, .byte2 = 0x00 },
};

static int cpu_test_opcode(struct opcode_info op_info, uint8_t mem[3])
{
	mem[0] = op_info.code;
	mem[1] = op_info.byte1;
	mem[2] = op_info.byte2;
	printf("\n");
	cpu_exec_opcode(mem[0], mem);
	return 0;
}

static int cpu_print_registers()
{
	printf("[REGS]  A=0x%X,      B=0x%X,    C=0x%X,    D=0x%X,    E=0x%X,    F=0x%X,    H=0x%X,    L=0x%X\n",
	       cpu_get_A(), cpu_get_B(), cpu_get_C(), cpu_get_D(), cpu_get_E(),
	       cpu_get_F(), cpu_get_H(), cpu_get_H());
	printf("[REGS]  AF=0x%04x,  BC=0x%04x, DE=0x%04x, HL=0x%04x\n",
	       cpu_get_AF(), cpu_get_BC(), cpu_get_DE(), cpu_get_HL());
	printf("[FLAGS] ZERO=%d,     SUB=%d,     HCARRY=%d,  CARRY=%d\n",
	       cpu_get_F() >> 7, (cpu_get_F() & 0x40) >> 6,
	       (cpu_get_F() & 0x20) >> 5, (cpu_get_F() & 0x10) >> 4);

	return 0;
}

static int cpu_print_test_result(struct opcode_info op_info,
				 int success_condition)
{
	if (!success_condition) {
		cpu_print_registers();
		printf("[ERROR] OPCODE 0x%X '%s' failed\n", op_info.code,
		       op_info.name);
		exit(0);
	} else {
		cpu_print_registers();
		printf("[SUCCESS] OPCODE 0x%X '%s' OK\n", op_info.code,
		       op_info.name);
	}
}

int testsuite_opcodes()
{
	printf("\n####################### CPU OPCODES UTEST #########################\n");

	uint8_t mem[3] = { 0x00, 0x00, 0x00 };
	uint8_t memfull[8000];
	uint8_t lg = 0;
	uint8_t opcode = 0x00;
	cpu_reset_registers();
	void cpu_reset_registers();
	memset(memfull, 0, 8000);

	// 0x01 : NOP
	opcode = 0x00;
	lg = cpu_exec_opcode(mem[0], mem);
	printf("[0x00][NOP] --------------------- mem={0x%X, 0x%X, 0x%X}\n",
	       mem[0], mem[1], mem[2]);

	// 0x01 : LD BC,d16
	opcode = 0x01;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_BC() == 0x4321);

	// LD (BC),A
	opcode = 0x02;
	cpu_reset_registers();
	cpu_set_A(0x42);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_BC() == 0x0042);

	// INC BC
	opcode = 0x03;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_BC() == 0x0001);

	// INC B
	opcode = 0x04;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x01);
	cpu_set_B(0xFF);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x00);

	// DEC B
	opcode = 0x05;
	cpu_reset_registers();
	cpu_set_B(0x01);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x00);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xFF);

	// LD B,d8
	opcode = 0x06;
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x42);

	// RLC A
	opcode = 0x07;
	cpu_reset_registers();
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x02);
	cpu_set_A(0xFF);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFE && cpu_get_F() == 0x10);

	// 0x08: LD (a16),SP
	opcode = 0x08;
	cpu_reset_registers();
	cpu_set_SP(0x42);
	memcpy(memfull, mem, 3);
	cpu_test_opcode(opcode_dict[opcode], memfull);
	printf("memfull[0x1234] = 0x%x\n", memfull[0x1234]);
	cpu_print_test_result(opcode_dict[opcode], memfull[0x1234] == 0x42);

	// 0x09: ADD HL,BC
	opcode = 0x09;
	cpu_reset_registers();
	cpu_set_HL(0x1111);
	cpu_set_BC(0x2222);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x3333);
	// test carry flag
	cpu_reset_registers();
	cpu_set_HL(0xF000);
	cpu_set_BC(0x2222);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x1222 && cpu_get_F() == 0x10);
	// test half carry flag
	cpu_reset_registers();
	cpu_set_HL(0x00FF);
	cpu_set_BC(0x0001);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x0100 && cpu_get_F() == 0x20);

	// 0x0A: LD LD A,(BC)
	opcode = 0x0A;
	cpu_reset_registers();
	cpu_set_BC(0x02);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x42);

	// 0x0B: DEC BC
	opcode = 0x0B;
	cpu_reset_registers();
	cpu_set_BC(0x77);
	cpu_test_opcode(opcode_dict[opcode], mem);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_BC() == 0x76);

	return 0;
}

typedef enum {
	FLAG_SET,
	FLAG_GET,
} cpu_flag_action;

int test_flag(cpu_flag_action action,
	      cpu_flag_name flag, cpu_flag_value value, uint8_t expected_value)
{
	switch (action) {
	case FLAG_SET:
		cpu_set_flag(flag, value);
		break;
	case FLAG_GET:
		break;
	}
	if (cpu_get_F() != expected_value) {
		printf("FLAG Register 'F' = 0x%x\n", cpu_get_F());
		printf("[ERROR][%s:%d] action %s failed for flag %d\n",
		       __func__, __LINE__, action == FLAG_SET ? "SET" : "GET",
		       flag);
		exit(0);
	} else {
		printf("[SUCCESS] FLAG action %s on flag %d\n",
		       action == FLAG_SET ? "SET" : "GET", flag);
	}

	return 0;
};

int testsuite_flags()
{
	printf("\n####################### CPU FLAGS UTEST #########################\n");

	//struct cpu_registers regs = {};
	cpu_reset_registers();

	printf("FLAG Register 'F' = 0x%x (init state)\n", cpu_get_F());

	test_flag(FLAG_SET, FLAG_ZERO, TRUE, 0x80);
	test_flag(FLAG_SET, FLAG_SUB, TRUE, 0xC0);
	test_flag(FLAG_SET, FLAG_HALF_CARRY, TRUE, 0xE0);
	test_flag(FLAG_SET, FLAG_CARRY, TRUE, 0xF0);

	test_flag(FLAG_SET, FLAG_ZERO, FALSE, 0x70);
	test_flag(FLAG_SET, FLAG_SUB, FALSE, 0x30);
	test_flag(FLAG_SET, FLAG_HALF_CARRY, FALSE, 0x10);
	test_flag(FLAG_SET, FLAG_CARRY, FALSE, 0x00);

	return 0;
}

int main()
{
	int ret = 0;

	ret = testsuite_flags();

	ret = testsuite_opcodes();

	return ret;
}