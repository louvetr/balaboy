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

static int test_opcode(struct opcode_info op_info, struct cpu_registers *regs,
		       uint8_t mem[3])
{
	mem[0] = op_info.code;
	mem[1] = op_info.byte1;
	mem[2] = op_info.byte2;
	printf("\n");
	exec_opcode(mem[0], regs, mem);
	return 0;
}

static int print_cpu_registers(struct cpu_registers regs)
{
	printf("[REGS]  A=0x%X,      B=0x%X,    C=0x%X,    D=0x%X,    E=0x%X,    F=0x%X,    H=0x%X,    L=0x%X\n",
	       regs.A, regs.B, regs.C, regs.D, regs.E, regs.F, regs.H, regs.L);
	printf("[REGS]  AF=0x%04x,  BC=0x%04x, DE=0x%04x, HL=0x%04x\n",
			get_AF(&regs), get_BC(&regs), get_DE(&regs), get_HL(&regs));
	       /**(uint16_t *)(&regs.A), *(uint16_t *)(&regs.B),
	       *(uint16_t *)(&regs.D), *(uint16_t *)(&regs.H));*/
	printf("[FLAGS] ZERO=%d,     SUB=%d,     HCARRY=%d,  CARRY=%d\n",
            regs.F >> 7,
            (regs.F & 0x40) >> 6,
            (regs.F & 0x20) >> 5,
            (regs.F & 0x10) >> 4);

	return 0;
}

static int print_cpu_test_result(struct opcode_info op_info, int success_condition,
				 struct cpu_registers regs)
{
	if (!success_condition) {
		print_cpu_registers(regs);
		printf("[ERROR] OPCODE 0x%X '%s' failed\n", op_info.code,
		       op_info.name);
		exit(0);
	} else {
		print_cpu_registers(regs);
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
	struct cpu_registers regs;
	memset(&regs, 0, sizeof(struct cpu_registers));
	memset(memfull, 0, 8000);

	// NOP
	opcode = 0x00;
	lg = exec_opcode(mem[0], &regs, mem);
	printf("[0x00][NOP] --------------------- mem={0x%X, 0x%X, 0x%X}\n",
	       mem[0], mem[1], mem[2]);

	// LD BC,d16
	opcode = 0x01;
	memset(&regs, 0, sizeof(struct cpu_registers));
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode],
			      *(uint16_t *)(&regs.B) == 0x4321, regs);

	// LD (BC),A
	opcode = 0x02;
	memset(&regs, 0, sizeof(struct cpu_registers));
	regs.A = 0x42;
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode],
			      *(uint16_t *)(&regs.B) == 0x0042, regs);

	// INC BC
	opcode = 0x03;
	memset(&regs, 0, sizeof(struct cpu_registers));
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode],
			      *(uint16_t *)(&regs.B) == 0x0001, regs);

	// INC B
	opcode = 0x04;
	memset(&regs, 0, sizeof(struct cpu_registers));
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.B == 0x01, regs);
    regs.B = 0xFF;
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.B == 0x00, regs);

	// DEC B
	opcode = 0x05;
	memset(&regs, 0, sizeof(struct cpu_registers));
    regs.B = 0x01;
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.B == 0x00, regs);
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.B == 0xFF, regs);

    // LD B,d8
	opcode = 0x06;
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.B == 0x42, regs);

    // RLC A
	opcode = 0x07;
	memset(&regs, 0, sizeof(struct cpu_registers));
    regs.A = 0x01;
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.A == 0x02, regs);
    regs.A = 0xFF;
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.A == 0xFE && regs.F == 0x10 , regs);

	// 0x08: LD (a16),SP	
	opcode = 0x08;
	memset(&regs, 0, sizeof(struct cpu_registers));
    regs.SP = 0x42;
	memcpy(memfull, mem, 3);
	test_opcode(opcode_dict[opcode], &regs, memfull);
	printf("memfull[0x1234] = 0x%x\n", memfull[0x1234]);
	print_cpu_test_result(opcode_dict[opcode], memfull[0x1234] == 0x42 , regs);

	// 0x09: ADD HL,BC	
	opcode = 0x09;
	memset(&regs, 0, sizeof(struct cpu_registers));
    set_HL(&regs, 0x1111);
    set_BC(&regs, 0x2222);
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], get_HL(&regs) == 0x3333 , regs);
	// test carry flag
	memset(&regs, 0, sizeof(struct cpu_registers));
    set_HL(&regs, 0xF000);
    set_BC(&regs, 0x2222);
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], get_HL(&regs) == 0x1222 && regs.F == 0x10 , regs);
	// test half carry flag
	memset(&regs, 0, sizeof(struct cpu_registers));
    set_HL(&regs, 0x00FF);
    set_BC(&regs, 0x0001);
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], get_HL(&regs) == 0x0100 && regs.F == 0x20 , regs);

	// 0x0A: LD LD A,(BC)	
	opcode = 0x0A;
	memset(&regs, 0, sizeof(struct cpu_registers));
    set_BC(&regs, 0x02);
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], regs.A == 0x42 , regs);

	// 0x0B: DEC BC	
	opcode = 0x0B;
	memset(&regs, 0, sizeof(struct cpu_registers));
    set_BC(&regs, 0x77);
	test_opcode(opcode_dict[opcode], &regs, mem);
	print_cpu_test_result(opcode_dict[opcode], get_BC(&regs) == 0x76 , regs);


	return 0;
}


typedef enum {
	FLAG_SET,
	FLAG_GET,
} cpu_flag_action;

int test_flag(struct cpu_registers *regs, cpu_flag_action action,
	      cpu_flag_name flag, cpu_flag_value value, uint8_t expected_value)
{
	switch (action) {
	case FLAG_SET:
		set_cpu_flag(regs, flag, value);
		break;
	case FLAG_GET:
		break;
	}
	if (regs->F != expected_value) {
	    printf("FLAG Register 'F' = 0x%x\n", regs->F);
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

	struct cpu_registers regs = {};

	printf("FLAG Register 'F' = 0x%x (init state)\n", regs.F);

	test_flag(&regs, FLAG_SET, FLAG_ZERO, TRUE, 0x80);
	test_flag(&regs, FLAG_SET, FLAG_SUB, TRUE, 0xC0);
	test_flag(&regs, FLAG_SET, FLAG_HALF_CARRY, TRUE, 0xE0);
	test_flag(&regs, FLAG_SET, FLAG_CARRY, TRUE, 0xF0);

	test_flag(&regs, FLAG_SET, FLAG_ZERO, FALSE, 0x70);
	test_flag(&regs, FLAG_SET, FLAG_SUB, FALSE, 0x30);
	test_flag(&regs, FLAG_SET, FLAG_HALF_CARRY, FALSE, 0x10);
	test_flag(&regs, FLAG_SET, FLAG_CARRY, FALSE, 0x00);

	return 0;
}

int main()
{
	int ret = 0;

	ret = testsuite_flags();

	ret = testsuite_opcodes();

	return ret;
}