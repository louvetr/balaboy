#include <errno.h>
#include <stdlib.h>
#include "cpu.h"
#include "memory.h"

struct opcode_info {
	uint8_t code;
	const char *name;
	uint8_t byte1;
	uint8_t byte2;
};

struct opcode_info opcode_dict[33] = {

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
	{.code = 0x0C, .name = "INC C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x0D, .name = "DEC C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x0E, .name = "LD C,d8", .byte1 = 0xAF, .byte2 = 0x00 },
	{.code = 0x0F, .name = "RRC A", .byte1 = 0x00, .byte2 = 0x00 },
	
	{.code = 0x10, .name = "STOP", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x11, .name = "LD DE,d16", .byte1 = 0x45, .byte2 = 0x67 },
	{.code = 0x12, .name = "LD (DE),A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x13, .name = "INC DE", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x14, .name = "INC D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x15, .name = "DEC D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x16, .name = "LD D,d8", .byte1 = 0x42, .byte2 = 0x00 },
	{.code = 0x17, .name = "RL A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x18, .name = "JR r8", .byte1 = 0xFF, .byte2 = 0x80 },
	{.code = 0x19, .name = "ADD HL,DE", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x1A, .name = "LD A,(DE)", .byte1 = 0x00, .byte2 = 0x42 },
	{.code = 0x1B, .name = "DEC DE", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x1C, .name = "INC E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x1D, .name = "DEC E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x1E, .name = "LD E,d8", .byte1 = 0xAF, .byte2 = 0x00 },
	{.code = 0x1F, .name = "RR A", .byte1 = 0x00, .byte2 = 0x00 },
	
	{.code = 0x20, .name = "JR NZ,r8", .byte1 = 0xFF, .byte2 = 0x80 },
};

static int cpu_test_opcode(struct opcode_info op_info)
{
	mem_set_byte(cpu_get_PC() + 0, op_info.code);
	mem_set_byte(cpu_get_PC() + 1, op_info.byte1);
	mem_set_byte(cpu_get_PC() + 2, op_info.byte2);
	printf("\n");
	return cpu_exec_opcode(mem_get_byte(cpu_get_PC()));
}

static int cpu_print_registers()
{
	printf("[REGS]  A=0x%X,      B=0x%X,    C=0x%X,    D=0x%X,    E=0x%X,    F=0x%X,    H=0x%X,    L=0x%X\n",
	       cpu_get_A(), cpu_get_B(), cpu_get_C(), cpu_get_D(), cpu_get_E(),
	       cpu_get_F(), cpu_get_H(), cpu_get_H());
	printf("[REGS]  AF=0x%04x,  BC=0x%04x, DE=0x%04x, HL=0x%04x, PC=0x%04x, SP=0x%04x\n",
	       cpu_get_AF(), cpu_get_BC(), cpu_get_DE(), cpu_get_HL(), cpu_get_PC(), cpu_get_SP());
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
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("[ERROR] OPCODE 0x%X '%s' failed\n", op_info.code,
		       op_info.name);
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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

	uint8_t memfull[8000];
	uint8_t lg = 0;
	uint8_t opcode = 0x00;
	cpu_reset_registers();
	void cpu_reset_registers();
	memset(memfull, 0, 8000);

	// 0x00 : NOP
	opcode = 0x00;
	lg = cpu_exec_opcode(mem_get_byte(0));

	// 0x01 : LD BC,d16
	opcode = 0x01;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_BC() == 0x4321);

	// 0x02 : LD (BC),A
	opcode = 0x02;
	cpu_reset_registers();
	cpu_set_A(0x42);
	cpu_set_BC(0x3333);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], mem_get_byte(0x3333) == 0x0042);

	// 0x03 : INC BC
	opcode = 0x03;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_BC() == 0x0001);

	// 0x04 : INC B
	opcode = 0x04;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x01);
	cpu_set_B(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x00);

	// 0x05 : DEC B
	opcode = 0x05;
	cpu_reset_registers();
	cpu_set_B(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xFF);

	// 0x06 : LD B,d8
	opcode = 0x06;
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x42);

	// 0x07 : RLC A
	opcode = 0x07;
	cpu_reset_registers();
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x02);
	cpu_set_A(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x10);

	// 0x08: LD (a16),SP
	opcode = 0x08;
	cpu_reset_registers();
	cpu_set_SP(0xABCD);
	//memcpy(memfull, mem, 3);
	cpu_test_opcode(opcode_dict[opcode]);
	printf("memfull[0x1234] = 0x%x\n", mem_get_byte(0x1234));
	cpu_print_test_result(opcode_dict[opcode], mem_get_byte(0x1234) == 0xCD && mem_get_byte(0x1235) == 0xAB);

	// 0x09: ADD HL,BC
	opcode = 0x09;
	cpu_reset_registers();
	cpu_set_HL(0x1111);
	cpu_set_BC(0x2222);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x3333);
	// test carry flag
	cpu_reset_registers();
	cpu_set_HL(0xF000);
	cpu_set_BC(0x2222);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x1222 && cpu_get_F() == 0x10);
	// test half carry flag
	cpu_reset_registers();
	cpu_set_HL(0x00FF);
	cpu_set_BC(0x0001);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x0100 && cpu_get_F() == 0x20);

	// 0x0A: LD LD A,(BC)
	opcode = 0x0A;
	cpu_reset_registers();
	cpu_set_BC(0x02);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x42);

	// 0x0B: DEC BC
	opcode = 0x0B;
	cpu_reset_registers();
	cpu_set_BC(0x77);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_BC() == 0x76);

	// 0x0C : INC C
	opcode = 0x0C;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0x01);
	cpu_set_C(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0x00);

	// 0x0D : DEC C
	opcode = 0x0D;
	cpu_reset_registers();
	cpu_set_C(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xFF);

	// 0x0E : LD C,d8
	opcode = 0x0E;
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xAF);

	// 0x0F : RRC A
	opcode = 0x0F;
	cpu_reset_registers();
	cpu_set_A(0x02);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x01 && cpu_get_F() == 0x00);
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x80 && cpu_get_F() == 0x10);
	cpu_set_A(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0x10 : STOP

	// 0x11 : LD DE,d16
	opcode = 0x11;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_DE() == 0x6745);

	// 0x12 : LD (DE), A
	opcode = 0x12;
	cpu_reset_registers();
	cpu_set_A(0x91);
	cpu_set_DE(0xFEDA);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], mem_get_byte(cpu_get_DE()) == 0x91);

	// 0x13 : INC DE
	opcode = 0x13;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_DE() == 0x0001);

	// 0x14 : INC D
	opcode = 0x14;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0x01);
	cpu_set_D(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0x00);

	// 0x15 : DEC D
	opcode = 0x15;
	cpu_reset_registers();
	cpu_set_D(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xFF);

	// 0x16 : LD D,d8
	opcode = 0x16;
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0x42);

	// 0x17 : RL A
	opcode = 0x17;
	cpu_reset_registers();
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x02);
	cpu_set_A(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFE && cpu_get_F() == 0x10);
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x03);

	// 0x18: JR r8
	printf("------------------- trying 0x18 : JR r8\n");
	opcode = 0x18;
	cpu_reset_registers();
	//cpu_set_PC(0x1000);
	cpu_set_PC(0x0800);
	//printf("[test] byte1 in mem = %d (0x%x)\n", (int8_t)mem[opcode].byte1);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	printf("int8_t byte1 = %d\n", (int8_t)opcode_dict[opcode].byte1);
	printf("int8_t byte2 = %d\n", (int8_t)opcode_dict[opcode].byte2);
	printf("[TEST] lg = %d\n", lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0801);

	// 0x19: ADD HL,DE
	opcode = 0x19;
	cpu_reset_registers();
	cpu_set_HL(0x1111);
	cpu_set_DE(0x2222);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x3333);
	// test carry flag
	cpu_reset_registers();
	cpu_set_HL(0xF000);
	cpu_set_DE(0x2222);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x1222 && cpu_get_F() == 0x10);
	// test half carry flag
	cpu_reset_registers();
	cpu_set_HL(0x00FF);
	cpu_set_DE(0x0001);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x0100 && cpu_get_F() == 0x20);

	// 0x1A: LD LD A,(DE)
	opcode = 0x1A;
	cpu_reset_registers();
	cpu_set_DE(0x02);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x42);

	// 0x1B: DEC DE
	opcode = 0x1B;
	cpu_reset_registers();
	cpu_set_DE(0x77);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_DE() == 0x76);

	// 0x1C : INC E
	opcode = 0x1C;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0x01);
	cpu_set_E(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0x00);

	// 0x1D : DEC E
	opcode = 0x1D;
	cpu_reset_registers();
	cpu_set_E(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xFF);

	// 0x1E : LD C,d8
	opcode = 0x1E;
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xAF);

	// 0x1F : RR A
	opcode = 0x1F;
	cpu_reset_registers();
	cpu_set_A(0x02);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x01 && cpu_get_F() == 0x00);
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x00 && cpu_get_F() == 0x90);
	cpu_set_A(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x80 && cpu_get_F() == 0x00);

	// 0x20: JR NZ, r8
	printf("------------------- trying 0x18 : JR r8\n");
	opcode = 0x20;
	cpu_reset_registers();
	cpu_set_PC(0x0800);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0801);
	cpu_set_PC(0x0800);
	cpu_set_F(0x80);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0802);


	printf("\n\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\n");
	printf("All OPCODES tests SUCCEED !!!\n");
	printf("\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\n");

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