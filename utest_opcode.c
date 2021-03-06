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

struct opcode_info opcode_dict[] = {

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
	{.code = 0x21, .name = "LD HL,d16", .byte1 = 0x45, .byte2 = 0x67 },
	{.code = 0x22, .name = "LD (HL+),A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x23, .name = "INC HL", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x24, .name = "INC H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x25, .name = "DEC H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x26, .name = "LD H,d8", .byte1 = 0x42, .byte2 = 0x00 },
	{.code = 0x27, .name = "DAA", .byte1 = 0x42, .byte2 = 0x00 },
	{.code = 0x28, .name = "JR Z,r8", .byte1 = 0xFF, .byte2 = 0x80 },
	{.code = 0x29, .name = "ADD HL,HL", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x2A, .name = "LD A,(HL+)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x2B, .name = "DEC HL", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x2C, .name = "INC L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x2D, .name = "DEC L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x2E, .name = "LD L,d8", .byte1 = 0x42, .byte2 = 0x00 },
	{.code = 0x2F, .name = "CPL", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0x30, .name = "JR NC,r8", .byte1 = 0xFF, .byte2 = 0x80 },
	{.code = 0x31, .name = "LD SP,d16", .byte1 = 0x45, .byte2 = 0x67 },
	{.code = 0x32, .name = "LD (HL-),A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x33, .name = "INC SP", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x34, .name = "INC (HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x35, .name = "DEC (HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x36, .name = "LD (HL),d8", .byte1 = 0xAA, .byte2 = 0x00 },
	{.code = 0x37, .name = "SCF", .byte1 = 0xFF, .byte2 = 0x00 },
	{.code = 0x38, .name = "JR C,r8", .byte1 = 0xFF, .byte2 = 0x00 },
	{.code = 0x39, .name = "ADD HL,SP", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x3A, .name = "LD A,(HL-)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x3B, .name = "DEC SP", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x3C, .name = "INC A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x3D, .name = "DEC A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x3E, .name = "LD A,d8", .byte1 = 0x42, .byte2 = 0x00 },
	{.code = 0x3F, .name = "CCF", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0x40, .name = "LD B,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x41, .name = "LD B,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x42, .name = "LD B,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x43, .name = "LD B,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x44, .name = "LD B,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x45, .name = "LD B,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x46, .name = "LD B,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x47, .name = "LD B,A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x48, .name = "LD C,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x49, .name = "LD C,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x4A, .name = "LD C,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x4B, .name = "LD C,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x4C, .name = "LD C,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x4D, .name = "LD C,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x4E, .name = "LD C,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x4F, .name = "LD C,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0x50, .name = "LD D,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x51, .name = "LD D,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x52, .name = "LD D,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x53, .name = "LD D,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x54, .name = "LD D,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x55, .name = "LD D,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x56, .name = "LD D,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x57, .name = "LD D,A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x58, .name = "LD E,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x59, .name = "LD E,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x5A, .name = "LD E,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x5B, .name = "LD E,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x5C, .name = "LD E,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x5D, .name = "LD E,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x5E, .name = "LD E,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x5F, .name = "LD E,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0x60, .name = "LD H,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x61, .name = "LD H,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x62, .name = "LD H,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x63, .name = "LD H,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x64, .name = "LD H,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x65, .name = "LD H,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x66, .name = "LD H,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x67, .name = "LD H,A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x68, .name = "LD L,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x69, .name = "LD L,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x6A, .name = "LD L,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x6B, .name = "LD L,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x6C, .name = "LD L,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x6D, .name = "LD L,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x6E, .name = "LD L,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x6F, .name = "LD L,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0x70, .name = "LD (HL),B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x71, .name = "LD (HL),C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x72, .name = "LD (HL),D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x73, .name = "LD (HL),E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x74, .name = "LD (HL),H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x75, .name = "LD (HL),L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x76, .name = "HALT", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x77, .name = "LD (HL),A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x78, .name = "LD A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x79, .name = "LD A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x7A, .name = "LD A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x7B, .name = "LD A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x7C, .name = "LD A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x7D, .name = "LD A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x7E, .name = "LD A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x7F, .name = "LD A,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0x80, .name = "ADD A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x81, .name = "ADD A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x82, .name = "ADD A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x83, .name = "ADD A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x84, .name = "ADD A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x85, .name = "ADD A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x86, .name = "ADD A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x87, .name = "ADD A,A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x88, .name = "ADC A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x89, .name = "ADC A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x8A, .name = "ADC A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x8B, .name = "ADC A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x8C, .name = "ADC A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x8D, .name = "ADC A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x8E, .name = "ADC A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x8F, .name = "ADC A,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0x90, .name = "SUB A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x91, .name = "SUB A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x92, .name = "SUB A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x93, .name = "SUB A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x94, .name = "SUB A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x95, .name = "SUB A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x96, .name = "SUB A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x97, .name = "SUB A,A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x98, .name = "SBC A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x99, .name = "SBC A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x9A, .name = "SBC A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x9B, .name = "SBC A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x9C, .name = "SBC A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x9D, .name = "SBC A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x9E, .name = "SBC A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0x9F, .name = "SBC A,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0xA0, .name = "AND A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA1, .name = "AND A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA2, .name = "AND A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA3, .name = "AND A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA4, .name = "AND A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA5, .name = "AND A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA6, .name = "AND A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA7, .name = "AND A,A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA8, .name = "XOR A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xA9, .name = "XOR A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xAA, .name = "XOR A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xAB, .name = "XOR A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xAC, .name = "XOR A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xAD, .name = "XOR A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xAE, .name = "XOR A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xAF, .name = "XOR A,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0xB0, .name = "OR A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB1, .name = "OR A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB2, .name = "OR A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB3, .name = "OR A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB4, .name = "OR A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB5, .name = "OR A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB6, .name = "OR A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB7, .name = "OR A,A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB8, .name = "CP A,B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xB9, .name = "CP A,C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xBA, .name = "CP A,D", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xBB, .name = "CP A,E", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xBC, .name = "CP A,H", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xBD, .name = "CP A,L", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xBE, .name = "CP A,(HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xBF, .name = "CP A,A", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0xC0, .name = "RET NZ", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xC1, .name = "POP BC", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xC2, .name = "JP NZ,a16", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xC3, .name = "JP a16", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xC4, .name = "CALL NZ,a16", .byte1 = 0x54, .byte2 = 0x76 },
	{.code = 0xC5, .name = "PUSH BC", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xC6, .name = "ADD A,d8", .byte1 = 0x33, .byte2 = 0x00 },
	{.code = 0xC7, .name = "RST 00h", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xC8, .name = "RET Z", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xC9, .name = "RET", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xCA, .name = "JP Z,a16", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xCB,
	 .name = "PREFIX CB",
	 .byte1 = 0x43,
	 .byte2 = 0x65 }, // STUB
	{.code = 0xCC, .name = "CALL Z,a16", .byte1 = 0x54, .byte2 = 0x76 },
	{.code = 0xCD, .name = "CALL a16", .byte1 = 0x54, .byte2 = 0x76 },
	{.code = 0xCE, .name = "ADC A,d8", .byte1 = 0x33, .byte2 = 0x00 },
	{.code = 0xCF, .name = "RST 08h", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0xD0, .name = "RET NC", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xD1, .name = "POP DE", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xD2, .name = "JP NC,a16", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xD3, .name = "NA", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xD4, .name = "CALL NC,a16", .byte1 = 0x54, .byte2 = 0x76 },
	{.code = 0xD5, .name = "PUSH DE", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xD6, .name = "SUB A,d8", .byte1 = 0x33, .byte2 = 0x00 },
	{.code = 0xD7, .name = "RST 10h", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xD8, .name = "RET C", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xD9, .name = "RETI", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xDA, .name = "JP C,a16", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xDB, .name = "NA", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xDC, .name = "CALL C,a16", .byte1 = 0x54, .byte2 = 0x76 },
	{.code = 0xDD, .name = "NA", .byte1 = 0x43, .byte2 = 0x65 },
	{.code = 0xDE, .name = "SBC A,d8", .byte1 = 0x33, .byte2 = 0x00 },
	{.code = 0xDF, .name = "RST 18h", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0xE0, .name = "LDH (a8),A", .byte1 = 0x11, .byte2 = 0x00 },
	{.code = 0xE1, .name = "POP HL", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xE2, .name = "LD (C),A", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xE3, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xE4, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xE5, .name = "PUSH HL", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xE6, .name = "AND d8", .byte1 = 0x33, .byte2 = 0x00 },
	{.code = 0xE7, .name = "RST 20h", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xE8, .name = "ADD SP,r8", .byte1 = 0x05, .byte2 = 0x00 },
	{.code = 0xE9, .name = "JP (HL)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xEA, .name = "LD (a16),A", .byte1 = 0x86, .byte2 = 0x54 },
	{.code = 0xEB, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xEC, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xED, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xEE, .name = "XOR d8", .byte1 = 0x0F, .byte2 = 0x00 },
	{.code = 0xEF, .name = "RST 28h", .byte1 = 0x00, .byte2 = 0x00 },

	{.code = 0xF0, .name = "LDH A,(a8)", .byte1 = 0x11, .byte2 = 0x00 },
	{.code = 0xF1, .name = "POP AF", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xF2, .name = "LD A,(C)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xF3, .name = "DI (no test)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xF4, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xF5, .name = "PUSH AF", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xF6, .name = "OR d8", .byte1 = 0x0F, .byte2 = 0x00 },
	{.code = 0xF7, .name = "RST 30h", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xF8, .name = "LD HL,SP+r8", .byte1 = 0x55, .byte2 = 0x00 },
	{.code = 0xF9, .name = "LD SP,HL", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xFA, .name = "LD A,(a16)", .byte1 = 0x51, .byte2 = 0x15 },
	{.code = 0xFB, .name = "EI (no test)", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xFC, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xFD, .name = "NA", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xFE, .name = "CP d8", .byte1 = 0x77, .byte2 = 0x00 },
	{.code = 0xFF, .name = "RST 38h", .byte1 = 0x00, .byte2 = 0x00 },

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
	       cpu_get_AF(), cpu_get_BC(), cpu_get_DE(), cpu_get_HL(),
	       cpu_get_PC(), cpu_get_SP());
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
	uint16_t u16;
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
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(0x3333) == 0x0042);

	// 0x03 : INC BC
	opcode = 0x03;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_BC() == 0x0001);

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
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(0x1234) == 0xCD &&
				      mem_get_byte(0x1235) == 0xAB);

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
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x01 && cpu_get_F() == 0x00);
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x80 && cpu_get_F() == 0x10);
	cpu_set_A(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

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
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_DE()) == 0x91);

	// 0x13 : INC DE
	opcode = 0x13;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_DE() == 0x0001);

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
	cpu_set_A(0xFE);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFC && cpu_get_F() == 0x10);
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
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x01 && cpu_get_F() == 0x00);
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x90);
	cpu_set_A(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x80 && cpu_get_F() == 0x00);

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

	// 0x21 : LD HL,d16
	opcode = 0x21;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x6745);

	// 0x22 : LD (HL+), A
	opcode = 0x22;
	cpu_reset_registers();
	cpu_set_A(0x91);
	cpu_set_HL(0xFEDA);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL() - 1) == 0x91 &&
				      cpu_get_HL() == 0xFEDB);

	// 0x23 : INC HL
	opcode = 0x23;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x0001);

	// 0x24 : INC H
	opcode = 0x24;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0x01);
	cpu_set_H(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0x00);

	// 0x25 : DEC H
	opcode = 0x25;
	cpu_reset_registers();
	cpu_set_H(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xFF);

	// 0x26 : LD H,d8
	opcode = 0x26;
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0x42);

	// 0x27 : DAA
	// examples founds on https://www.tutorialspoint.com/daa-instruction-in-8085-microprocessor
	opcode = 0x27;
	cpu_reset_registers();
	cpu_set_A(0x7D);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x83);
	cpu_reset_registers();
	cpu_set_A(0x79);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x79);
	cpu_reset_registers();
	cpu_set_A(0xD7);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x37);
	cpu_reset_registers();
	cpu_set_A(0xCC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x32);
	// TODO: check all IF cases from function

	// 0x28: JR Z, r8
	opcode = 0x28;
	cpu_reset_registers();
	cpu_set_PC(0x0800);
	cpu_set_F(0x80);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0801);
	cpu_reset_registers();
	cpu_set_PC(0x0800);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0802);

	// 0x29: ADD HL,HL
	opcode = 0x29;
	cpu_reset_registers();
	cpu_set_HL(0x1111);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x2222);
	// test carry flag
	cpu_reset_registers();
	cpu_set_HL(0xF000);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0xE000 && cpu_get_F() == 0x10);
	// test half carry flag
	cpu_reset_registers();
	cpu_set_HL(0x00FF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x01FE && cpu_get_F() == 0x20);

	// 0x2A : LD A, (HL+)
	opcode = 0x2A;
	cpu_reset_registers();
	cpu_set_HL(0xFEDA);
	mem_set_byte(cpu_get_HL(), 0x33);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x33 && cpu_get_HL() == 0xFEDB);

	// 0x2B: DEC HL
	opcode = 0x2B;
	cpu_reset_registers();
	cpu_set_HL(0x77);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x76);

	// 0x2C : INC L
	opcode = 0x2C;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0x01);
	cpu_set_L(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0x00);

	// 0x2D : DEC L
	opcode = 0x2D;
	cpu_reset_registers();
	cpu_set_L(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xFF);

	// 0x2E : LD L,d8
	opcode = 0x2E;
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0x42);

	// 0x2F : CPL
	opcode = 0x2F;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);

	// 0x30: JR NC, r8
	printf("------------------- trying 0x18 : JR r8\n");
	opcode = 0x30;
	cpu_reset_registers();
	cpu_set_PC(0x0800);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0801);
	cpu_set_PC(0x0800);
	cpu_set_F(0x10);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0802);

	// 0x31 : LD SP,d16
	opcode = 0x31;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x6745);

	// 0x32 : LD (HL-), A
	opcode = 0x32;
	cpu_reset_registers();
	cpu_set_A(0x91);
	cpu_set_HL(0xFEDA);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL() + 1) == 0x91 &&
				      cpu_get_HL() == 0xFED9);

	// 0x33 : INC SP
	opcode = 0x33;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x0001);

	// 0x34 : INC (HL)
	opcode = 0x34;
	cpu_reset_registers();
	cpu_set_HL(0x5555);
	mem_set_byte(0x5555, 0x66);
	cpu_test_opcode(opcode_dict[opcode]);
	printf("mem[0x%x] = 0x%x\n", cpu_get_HL(), mem_get_byte(cpu_get_HL()));
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0x67);
	cpu_set_HL(0x5555);
	mem_set_byte(0x5555, 0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	printf("mem[0x%x] = 0x%x\n", cpu_get_HL(), mem_get_byte(cpu_get_HL()));
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0x00 &&
				      cpu_get_F() == 0xA0);

	// 0x35 : DEC (HL)
	opcode = 0x35;
	cpu_reset_registers();
	cpu_set_HL(0x5555);
	mem_set_byte(0x5555, 0x66);
	cpu_test_opcode(opcode_dict[opcode]);
	printf("mem[0x%x] = 0x%x\n", cpu_get_HL(), mem_get_byte(cpu_get_HL()));
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0x65);
	cpu_set_HL(0x5555);
	mem_set_byte(0x5555, 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	printf("mem[0x%x] = 0x%x\n", cpu_get_HL(), mem_get_byte(cpu_get_HL()));
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xFF &&
				      cpu_get_F() == 0x20);

	// 0x36 : LD (HL), d8
	opcode = 0x36;
	cpu_reset_registers();
	cpu_set_HL(0xFEDA);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xAA &&
				      cpu_get_HL() == 0xFEDA);

	// 0x37 : SCF
	opcode = 0x37;
	cpu_reset_registers();
	cpu_set_F(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x90);

	// 0x38: JR C, r8
	opcode = 0x38;
	cpu_reset_registers();
	cpu_set_PC(0x0800);
	cpu_set_F(0x10);
	printf("mem[0x%x] = 0x%x\n", cpu_get_HL(), mem_get_byte(cpu_get_HL()));
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0801);
	cpu_set_PC(0x0800);
	cpu_set_F(0x00);
	lg = cpu_test_opcode(opcode_dict[opcode]);
	cpu_set_PC(cpu_get_PC() + lg);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0802);

	// 0x39: ADD HL,SP
	opcode = 0x39;
	cpu_reset_registers();
	cpu_set_HL(0x1111);
	cpu_set_SP(0x2222);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x3333);
	// test carry flag
	cpu_reset_registers();
	cpu_set_HL(0xF000);
	cpu_set_SP(0xFFFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0xEFFF && cpu_get_F() == 0x10);
	// test half carry flag
	cpu_reset_registers();
	cpu_set_HL(0x00FF);
	cpu_set_SP(0x0001);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x0100 && cpu_get_F() == 0x20);

	// 0x3A : LD A, (HL-)
	opcode = 0x3A;
	cpu_reset_registers();
	cpu_set_HL(0xFEDA);
	mem_set_byte(cpu_get_HL(), 0x33);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x33 && cpu_get_HL() == 0xFED9);

	// 0x3B: DEC SP
	opcode = 0x3B;
	cpu_reset_registers();
	cpu_set_SP(0x77);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x76);

	// 0x3C : INC A
	opcode = 0x3C;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x01);
	cpu_set_A(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x00);

	// 0x3D : DEC A
	opcode = 0x3D;
	cpu_reset_registers();
	cpu_set_A(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0x3E : LD A,d8
	opcode = 0x3E;
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x42);

	// 0x3F : SCF
	opcode = 0x3F;
	cpu_reset_registers();
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x10);

	// 0x40 : LD B,B
	opcode = 0x40;
	cpu_reset_registers();
	cpu_set_B(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xDC);

	// 0x41 : LD B,C
	opcode = 0x41;
	cpu_reset_registers();
	cpu_set_C(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xDC);

	// 0x42 : LD B,D
	opcode = 0x42;
	cpu_reset_registers();
	cpu_set_D(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xDC);

	// 0x43 : LD B,E
	opcode = 0x43;
	cpu_reset_registers();
	cpu_set_E(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xDC);

	// 0x44 : LD B,H
	opcode = 0x44;
	cpu_reset_registers();
	cpu_set_H(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xDC);

	// 0x45 : LD B,L
	opcode = 0x45;
	cpu_reset_registers();
	cpu_set_L(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xDC);

	// 0x46 : LD B, (HL)
	opcode = 0x46;
	cpu_reset_registers();
	cpu_set_HL(0xAAAA);
	mem_set_byte(cpu_get_HL(), 0x33);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0x33);

	// 0x47 : LD B,A
	opcode = 0x47;
	cpu_reset_registers();
	cpu_set_A(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_B() == 0xDC);

	// 0x48 : LD C,B
	opcode = 0x48;
	cpu_reset_registers();
	cpu_set_B(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xDC);

	// 0x49 : LD C,C
	opcode = 0x49;
	cpu_reset_registers();
	cpu_set_C(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xDC);

	// 0x4A : LD C,D
	opcode = 0x4A;
	cpu_reset_registers();
	cpu_set_D(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xDC);

	// 0x4B : LD C,E
	opcode = 0x4B;
	cpu_reset_registers();
	cpu_set_E(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xDC);

	// 0x4C : LD C,H
	opcode = 0x4C;
	cpu_reset_registers();
	cpu_set_H(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xDC);

	// 0x4D : LD C,L
	opcode = 0x4D;
	cpu_reset_registers();
	cpu_set_L(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xDC);

	// 0x4E : LD C, (HL)
	opcode = 0x4E;
	cpu_reset_registers();
	cpu_set_HL(0xBBBB);
	mem_set_byte(cpu_get_HL(), 0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0x44);

	// 0x4F : LD C,A
	opcode = 0x4F;
	cpu_reset_registers();
	cpu_set_A(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_C() == 0xDC);

	// 0x50 : LD D,B
	opcode = 0x50;
	cpu_reset_registers();
	cpu_set_B(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xDC);

	// 0x51 : LD D,C
	opcode = 0x51;
	cpu_reset_registers();
	cpu_set_C(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xDC);

	// 0x52 : LD B,D
	opcode = 0x52;
	cpu_reset_registers();
	cpu_set_D(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xDC);

	// 0x53 : LD D,E
	opcode = 0x53;
	cpu_reset_registers();
	cpu_set_E(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xDC);

	// 0x54 : LD D,H
	opcode = 0x54;
	cpu_reset_registers();
	cpu_set_H(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xDC);

	// 0x55 : LD D,L
	opcode = 0x55;
	cpu_reset_registers();
	cpu_set_L(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xDC);

	// 0x56 : LD D, (HL)
	opcode = 0x56;
	cpu_reset_registers();
	cpu_set_HL(0xAAAA);
	mem_set_byte(cpu_get_HL(), 0x33);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0x33);

	// 0x57 : LD D,A
	opcode = 0x57;
	cpu_reset_registers();
	cpu_set_A(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_D() == 0xDC);

	// 0x58 : LD E,B
	opcode = 0x58;
	cpu_reset_registers();
	cpu_set_B(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xDC);

	// 0x59 : LD E,C
	opcode = 0x59;
	cpu_reset_registers();
	cpu_set_C(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xDC);

	// 0x5A : LD E,D
	opcode = 0x5A;
	cpu_reset_registers();
	cpu_set_D(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xDC);

	// 0x5B : LD E,E
	opcode = 0x5B;
	cpu_reset_registers();
	cpu_set_E(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xDC);

	// 0x5C : LD E,H
	opcode = 0x5C;
	cpu_reset_registers();
	cpu_set_H(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xDC);

	// 0x5D : LD E,L
	opcode = 0x5D;
	cpu_reset_registers();
	cpu_set_L(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xDC);

	// 0x5E : LD C, (HL)
	opcode = 0x5E;
	cpu_reset_registers();
	cpu_set_HL(0xBBBB);
	mem_set_byte(cpu_get_HL(), 0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0x44);

	// 0x5F : LD E,A
	opcode = 0x5F;
	cpu_reset_registers();
	cpu_set_A(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_E() == 0xDC);

	// 0x60 : LD H,B
	opcode = 0x60;
	cpu_reset_registers();
	cpu_set_B(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xDC);

	// 0x61 : LD H,C
	opcode = 0x61;
	cpu_reset_registers();
	cpu_set_C(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xDC);

	// 0x62 : LD H,D
	opcode = 0x62;
	cpu_reset_registers();
	cpu_set_D(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xDC);

	// 0x63 : LD H,E
	opcode = 0x63;
	cpu_reset_registers();
	cpu_set_E(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xDC);

	// 0x64 : LD H,H
	opcode = 0x64;
	cpu_reset_registers();
	cpu_set_H(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xDC);

	// 0x65 : LD H,L
	opcode = 0x65;
	cpu_reset_registers();
	cpu_set_L(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xDC);

	// 0x66 : LD H, (HL)
	opcode = 0x66;
	cpu_reset_registers();
	cpu_set_HL(0xAAAA);
	mem_set_byte(cpu_get_HL(), 0x33);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0x33);

	// 0x67 : LD H,A
	opcode = 0x67;
	cpu_reset_registers();
	cpu_set_A(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_H() == 0xDC);

	// 0x68 : LD L,B
	opcode = 0x68;
	cpu_reset_registers();
	cpu_set_B(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xDC);

	// 0x69 : LD L,C
	opcode = 0x69;
	cpu_reset_registers();
	cpu_set_C(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xDC);

	// 0x6A : LD L,D
	opcode = 0x6A;
	cpu_reset_registers();
	cpu_set_D(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xDC);

	// 0x6B : LD L,E
	opcode = 0x6B;
	cpu_reset_registers();
	cpu_set_E(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xDC);

	// 0x6C : LD L,H
	opcode = 0x6C;
	cpu_reset_registers();
	cpu_set_H(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xDC);

	// 0x6D : LD L,L
	opcode = 0x6D;
	cpu_reset_registers();
	cpu_set_L(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xDC);

	// 0x6E : LD L, (HL)
	opcode = 0x6E;
	cpu_reset_registers();
	cpu_set_HL(0xBBBB);
	mem_set_byte(cpu_get_HL(), 0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0x44);

	// 0x6F : LD L,A
	opcode = 0x6F;
	cpu_reset_registers();
	cpu_set_A(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_L() == 0xDC);

	// 0x70 : LD (HL),B
	opcode = 0x70;
	cpu_reset_registers();
	cpu_set_HL(0xABD0);
	cpu_set_B(0xDD);
	mem_set_byte(cpu_get_HL(), 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDD);

	// 0x71 : LD (HL),C
	opcode = 0x71;
	cpu_reset_registers();
	cpu_set_HL(0xABD0);
	cpu_set_C(0xDD);
	mem_set_byte(cpu_get_HL(), 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDD);

	// 0x72 : LD (HL),D
	opcode = 0x72;
	cpu_reset_registers();
	cpu_set_HL(0xABD0);
	cpu_set_D(0xDD);
	mem_set_byte(cpu_get_HL(), 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDD);

	// 0x73 : LD (HL),E
	opcode = 0x73;
	cpu_reset_registers();
	cpu_set_HL(0xABD0);
	cpu_set_E(0xDD);
	mem_set_byte(cpu_get_HL(), 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDD);

	// 0x74 : LD (HL),H
	opcode = 0x74;
	cpu_reset_registers();
	cpu_set_HL(0xABD0);
	cpu_set_H(0xDD);
	mem_set_byte(cpu_get_HL(), 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDD);

	// 0x75 : LD (HL),L
	opcode = 0x75;
	cpu_reset_registers();
	cpu_set_HL(0xABD0);
	cpu_set_L(0xDD);
	mem_set_byte(cpu_get_HL(), 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDD);

	// 0x77 : LD (HL),A
	opcode = 0x77;
	cpu_reset_registers();
	cpu_set_HL(0xABD0);
	cpu_set_A(0xDD);
	mem_set_byte(cpu_get_HL(), 0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDD);

	// 0x78 : LD A,B
	opcode = 0x78;
	cpu_reset_registers();
	cpu_set_B(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xDC);

	// 0x79 : LD A,C
	opcode = 0x79;
	cpu_reset_registers();
	cpu_set_C(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xDC);

	// 0x7A : LD A,D
	opcode = 0x7A;
	cpu_reset_registers();
	cpu_set_D(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xDC);

	// 0x7B : LD A,E
	opcode = 0x7B;
	cpu_reset_registers();
	cpu_set_E(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xDC);

	// 0x7C : LD A,H
	opcode = 0x7C;
	cpu_reset_registers();
	cpu_set_H(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xDC);

	// 0x7D : LD A,L
	opcode = 0x7D;
	cpu_reset_registers();
	cpu_set_L(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xDC);

	// 0x7E : LD A, (HL)
	opcode = 0x7E;
	cpu_reset_registers();
	cpu_set_HL(0xDC12);
	mem_set_byte(cpu_get_HL(), 0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x44);

	// 0x7F : LD A,A
	opcode = 0x7F;
	cpu_reset_registers();
	cpu_set_A(0xDC);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xDC);

	// 0x80 : ADD A,B
	opcode = 0x80;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_B(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0F);
	cpu_set_B(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_B(0x01);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x81 : ADD A,C
	opcode = 0x81;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_C(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0F);
	cpu_set_C(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_C(0x01);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x82 : ADD A,D
	opcode = 0x82;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_D(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0F);
	cpu_set_D(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_D(0x01);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x83 : ADD A,E
	opcode = 0x83;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_E(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0F);
	cpu_set_E(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_E(0x01);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x84 : ADD A,H
	opcode = 0x84;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_H(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0F);
	cpu_set_H(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_H(0x01);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x85 : ADD A,L
	opcode = 0x85;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_L(0x01);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0F);
	cpu_set_L(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_L(0x01);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x86 : ADD A,(HL)
	opcode = 0x86;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_HL(0x8525);
	mem_set_byte(cpu_get_HL(), 0x71);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x7A);

	// 0x87 : ADD A,A
	opcode = 0x87;
	cpu_reset_registers();
	cpu_set_A(0x03);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x06);
	cpu_set_A(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFE && cpu_get_F() == 0x30);

	// 0x88 : ADC A,B
	opcode = 0x88;
	cpu_reset_registers();
	cpu_set_A(0x08);
	cpu_set_B(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0E);
	cpu_set_B(0x0F);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFE);
	cpu_set_B(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x89 : ADC A,C
	opcode = 0x89;
	cpu_reset_registers();
	cpu_set_A(0x08);
	cpu_set_C(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0E);
	cpu_set_C(0x0F);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFE);
	cpu_set_C(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x8A : ADC A,D
	opcode = 0x8A;
	cpu_reset_registers();
	cpu_set_A(0x08);
	cpu_set_D(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0E);
	cpu_set_D(0x0F);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFE);
	cpu_set_D(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x8B : ADC A,E
	opcode = 0x8B;
	cpu_reset_registers();
	cpu_set_A(0x08);
	cpu_set_E(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0E);
	cpu_set_E(0x0F);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFE);
	cpu_set_E(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x8C : ADC A,H
	opcode = 0x8C;
	cpu_reset_registers();
	cpu_set_A(0x08);
	cpu_set_H(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0E);
	cpu_set_H(0x0F);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFE);
	cpu_set_H(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x8D : ADC A,L
	opcode = 0x8D;
	cpu_reset_registers();
	cpu_set_A(0x08);
	cpu_set_L(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0A);
	cpu_set_A(0x0E);
	cpu_set_L(0x0F);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFE);
	cpu_set_L(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xB0);

	// 0x8E : ADC A,(HL)
	opcode = 0x8E;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_HL(0x8525);
	mem_set_byte(cpu_get_HL(), 0x71);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x7A);

	// 0x8F : ADC A,A
	opcode = 0x8F;
	cpu_reset_registers();
	cpu_set_A(0x03);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x06);
	cpu_set_A(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x1E && cpu_get_F() == 0x20);
	cpu_set_A(0xFF);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFE && cpu_get_F() == 0x30);

	// 0x90 : SUB A,B
	opcode = 0x90;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_B(0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_B(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_B(0x05);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x91 : SUB A,C
	opcode = 0x91;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_C(0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_C(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_C(0x05);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x92 : SUB A,D
	opcode = 0x92;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_D(0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_D(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_D(0x05);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x93 : SUB A,E
	opcode = 0x93;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_E(0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_E(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_E(0x05);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x94 : SUB A,H
	opcode = 0x94;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_H(0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_H(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_H(0x05);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x95 : SUB A,L
	opcode = 0x95;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_L(0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_L(0x0F);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_L(0x05);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x96 : SUB A,(HL)
	opcode = 0x96;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_HL(0x7531);
	mem_set_byte(cpu_get_HL(), 0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);

	// 0x97 : SUB A,A
	opcode = 0x97;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);

	// 0x98 : SBC A,B
	opcode = 0x98;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_B(0x08);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_B(0x0E);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_B(0x04);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x99 : SBC A,C
	opcode = 0x99;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_C(0x08);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_C(0x0E);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_C(0x04);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x9A : SBC A,D
	opcode = 0x9A;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_D(0x08);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_D(0x0E);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_D(0x04);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x9B : SBC A,E
	opcode = 0x9B;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_E(0x08);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_E(0x0E);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_E(0x04);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x9C : SBC A,H
	opcode = 0x9C;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_H(0x08);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_H(0x0E);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_H(0x04);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x9D : SBC A,L
	opcode = 0x9D;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_L(0x08);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);
	cpu_set_A(0xFE);
	cpu_set_L(0x0E);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xEF && cpu_get_F() == 0x60);
	cpu_set_A(0x04);
	cpu_set_L(0x04);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0x9E : SBC A,(HL)
	opcode = 0x9E;
	cpu_reset_registers();
	cpu_set_A(0x09);
	cpu_set_HL(0x7531);
	mem_set_byte(cpu_get_HL(), 0x08);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xC0);

	// 0x9F : SBC A,A
	opcode = 0x9F;
	cpu_reset_registers();
	cpu_set_A(0x01);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x70);

	// 0xA0 : AND A,B
	opcode = 0xA0;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_set_B(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);
	cpu_set_A(0xFF);
	cpu_set_B(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xA0);

	// 0xA1 : AND A,C
	opcode = 0xA1;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_set_C(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);
	cpu_set_A(0xFF);
	cpu_set_C(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xA0);

	// 0xA2 : AND A,D
	opcode = 0xA2;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_set_D(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);
	cpu_set_A(0xFF);
	cpu_set_D(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xA0);

	// 0xA3 : AND A,E
	opcode = 0xA3;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_set_E(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);
	cpu_set_A(0xFF);
	cpu_set_E(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xA0);

	// 0xA4 : AND A,H
	opcode = 0xA4;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_set_H(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);
	cpu_set_A(0xFF);
	cpu_set_H(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xA0);

	// 0xA5 : AND A,L
	opcode = 0xA5;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_set_L(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);
	cpu_set_A(0xFF);
	cpu_set_L(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0xA0);

	// 0xA6 : AND A,(HL)
	opcode = 0xA6;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_set_HL(0xDADA);
	mem_set_byte(cpu_get_HL(), 0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x0F);

	// 0xA7 : AND A,A
	opcode = 0xA7;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0xFF && cpu_get_F() == 0x20);

	// 0xA8 : XOR A,B
	opcode = 0xA8;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_set_B(0x15);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x3E);
	cpu_set_A(0xF0);
	cpu_set_B(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0xA9 : XOR A,C
	opcode = 0xA9;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_set_C(0x15);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x3E);
	cpu_set_A(0xF0);
	cpu_set_C(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0xAA : XOR A,D
	opcode = 0xAA;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_set_D(0x15);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x3E);
	cpu_set_A(0xF0);
	cpu_set_D(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0xAB : XOR A,E
	opcode = 0xAB;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_set_E(0x15);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x3E);
	cpu_set_A(0xF0);
	cpu_set_E(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0xAC : XOR A,H
	opcode = 0xAC;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_set_H(0x15);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x3E);
	cpu_set_A(0xF0);
	cpu_set_H(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0xAD : XOR A,L
	opcode = 0xAD;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_set_L(0x15);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x3E);
	cpu_set_A(0xF0);
	cpu_set_L(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0xAE : XOR A,(HL)
	opcode = 0xAE;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_set_HL(0xDADA);
	mem_set_byte(cpu_get_HL(), 0x15);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x3E);

	// 0xAF : XOR A,A
	opcode = 0xAF;
	cpu_reset_registers();
	cpu_set_A(0x2B);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_A() == 0x00 && cpu_get_F() == 0x80);

	// 0xB0 : AND A,B
	opcode = 0xB0;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_set_B(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xB1 : AND A,C
	opcode = 0xB1;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_set_C(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xB2 : AND A,D
	opcode = 0xB2;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_set_D(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xB3 : AND A,E
	opcode = 0xB3;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_set_E(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xB4 : AND A,H
	opcode = 0xB4;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_set_H(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xB5 : AND A,L
	opcode = 0xB5;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_set_L(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xB6 : AND A,(HL)
	opcode = 0xB6;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_set_HL(0x8527);
	mem_set_byte(cpu_get_HL(), 0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xB7 : AND A,A
	opcode = 0xB;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xF0);

	// 0xB8 : CP A,B
	opcode = 0xB8;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_B(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_B(0x2F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x50);
	cpu_reset_registers();
	cpu_set_A(0xF1);
	cpu_set_B(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x60);

	// 0xB9 : CP A,C
	opcode = 0xB9;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_C(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_C(0x2F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x50);
	cpu_reset_registers();
	cpu_set_A(0xF1);
	cpu_set_C(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x60);

	// 0xBA : CP A,D
	opcode = 0xBA;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_D(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_D(0x2F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x50);
	cpu_reset_registers();
	cpu_set_A(0xF1);
	cpu_set_D(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x60);

	// 0xBB : CP A,E
	opcode = 0xBB;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_E(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_E(0x2F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x50);
	cpu_reset_registers();
	cpu_set_A(0xF1);
	cpu_set_E(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x60);

	// 0xBC : CP A,H
	opcode = 0xBC;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_H(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_H(0x2F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x50);
	cpu_reset_registers();
	cpu_set_A(0xF1);
	cpu_set_H(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x60);

	// 0xBD : CP A,L
	opcode = 0xBD;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_L(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_L(0x2F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x50);
	cpu_reset_registers();
	cpu_set_A(0xF1);
	cpu_set_L(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x60);

	// 0xBE : CP A,(HL)
	opcode = 0xBE;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_set_HL(0x1511);
	mem_set_byte(cpu_get_HL(), 0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);

	// 0xBF : CP A,A
	opcode = 0xBF;
	cpu_reset_registers();
	cpu_set_A(0x1F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);

	// 0xC0 : RT NZ
	opcode = 0xC0;
	cpu_reset_registers();
	cpu_set_F(0x80);
	cpu_set_SP(0x0FFE);
	cpu_set_PC(0x5432);
	mem_set_byte(cpu_get_SP(), 0xCA);
	mem_set_byte(cpu_get_SP() + 1, 0xDB);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x5432 && cpu_get_SP() == 0x0FFE);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xDBCA && cpu_get_SP() == 0x1000);

	// 0xC1 : POP BC
	opcode = 0xC1;
	cpu_reset_registers();
	cpu_set_SP(0x0FFE);
	mem_set_byte(cpu_get_SP(), 0x21);
	mem_set_byte(cpu_get_SP() + 1, 0x43);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_BC() == 0x4321 && cpu_get_SP() == 0x1000);

	// 0xC2 : JP NZ,a16
	opcode = 0xC2;
	cpu_reset_registers();
	cpu_set_F(0x80);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0000);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x6543);

	// 0xC3 : JP a16
	opcode = 0xC3;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x6543);

	// 0xC4 : CALL NZ,a16
	opcode = 0xC4;
	cpu_reset_registers();
	cpu_set_F(0x80);
	cpu_set_SP(0x1000);
	cpu_set_PC(0xABCD);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xABCD && cpu_get_SP() == 0x1000);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x7654 && cpu_get_SP() == 0x0FFE);

	// 0xC5 : PUSH BC
	opcode = 0xC5;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_BC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x0FFE);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xC6 : ADD A,d8
	opcode = 0xC6;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x33);

	// 0xC7 : RST 00h
	opcode = 0xC7;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0000);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xC8 : RET Z
	opcode = 0xC8;
	cpu_reset_registers();
	cpu_set_F(0x00);
	cpu_set_SP(0x0FFE);
	cpu_set_PC(0x5432);
	mem_set_byte(cpu_get_SP(), 0xCA);
	mem_set_byte(cpu_get_SP() + 1, 0xDB);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x5432 && cpu_get_SP() == 0x0FFE);
	cpu_set_F(0x80);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xDBCA && cpu_get_SP() == 0x1000);

	// 0xC9 : RET
	opcode = 0xC9;
	cpu_reset_registers();
	cpu_set_SP(0x0FFE);
	cpu_set_PC(0x5432);
	mem_set_byte(cpu_get_SP(), 0xCA);
	mem_set_byte(cpu_get_SP() + 1, 0xDB);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xDBCA && cpu_get_SP() == 0x1000);

	// 0xCA : JP Z,a16
	opcode = 0xCA;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0000);
	cpu_set_F(0x80);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x6543);

	// 0xCC : CALL Z,a16
	opcode = 0xCC;
	cpu_reset_registers();
	cpu_set_F(0x00);
	cpu_set_SP(0x1000);
	cpu_set_PC(0xABCD);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xABCD && cpu_get_SP() == 0x1000);
	cpu_set_F(0x80);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x7654 && cpu_get_SP() == 0x0FFE);

	// 0xCD : CALL a16
	opcode = 0xCD;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xABCD);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x7654 && cpu_get_SP() == 0x0FFE);

	// 0xCE : ADC A,d8
	opcode = 0xCE;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x33);

	// 0xCF : RST 08h
	opcode = 0xCF;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0008);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xD0 : RET NC
	opcode = 0xD0;
	cpu_reset_registers();
	cpu_set_F(0x10);
	cpu_set_SP(0x0FFE);
	cpu_set_PC(0x5432);
	mem_set_byte(cpu_get_SP(), 0xCA);
	mem_set_byte(cpu_get_SP() + 1, 0xDB);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x5432 && cpu_get_SP() == 0x0FFE);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xDBCA && cpu_get_SP() == 0x1000);

	// 0xD1 : POP DE
	opcode = 0xD1;
	cpu_reset_registers();
	cpu_set_SP(0x0FFE);
	mem_set_byte(cpu_get_SP(), 0x21);
	mem_set_byte(cpu_get_SP() + 1, 0x43);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_DE() == 0x4321 && cpu_get_SP() == 0x1000);

	// 0xD2 : JP NC,a16
	opcode = 0xD2;
	cpu_reset_registers();
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0000);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x6543);

	// 0xD4 : CALL NC,a16
	opcode = 0xD4;
	cpu_reset_registers();
	cpu_set_F(0x10);
	cpu_set_SP(0x1000);
	cpu_set_PC(0xABCD);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xABCD && cpu_get_SP() == 0x1000);
	cpu_set_F(0x00);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x7654 && cpu_get_SP() == 0x0FFE);

	// 0xD5 : PUSH DE
	opcode = 0xD5;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_DE(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x0FFE);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xD6 : ADD A,d8
	opcode = 0xD6;
	cpu_reset_registers();
	cpu_set_A(0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x11);

	// 0xD7 : RST 10h
	opcode = 0xD7;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0010);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xD8 : RET C
	opcode = 0xD8;
	cpu_reset_registers();
	cpu_set_F(0x00);
	cpu_set_SP(0x0FFE);
	cpu_set_PC(0x5432);
	mem_set_byte(cpu_get_SP(), 0xCA);
	mem_set_byte(cpu_get_SP() + 1, 0xDB);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x5432 && cpu_get_SP() == 0x0FFE);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xDBCA && cpu_get_SP() == 0x1000);

	// 0xD9 : RETI
	opcode = 0xD9;
	cpu_reset_registers();
	cpu_set_SP(0x0FFE);
	cpu_set_PC(0x5432);
	mem_set_byte(cpu_get_SP(), 0xCA);
	mem_set_byte(cpu_get_SP() + 1, 0xDB);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xDBCA && cpu_get_SP() == 0x1000);

	// 0xDA : JP C,a16
	opcode = 0xDA;
	cpu_reset_registers();
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x0000);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x6543);

	// 0xDC : CALL C,a16
	opcode = 0xDC;
	cpu_reset_registers();
	cpu_set_F(0x00);
	cpu_set_SP(0x1000);
	cpu_set_PC(0xABCD);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0xABCD && cpu_get_SP() == 0x1000);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_PC() == 0x7654 && cpu_get_SP() == 0x0FFE);

	// 0xDE : SDC A,d8
	opcode = 0xDE;
	cpu_reset_registers();
	cpu_set_A(0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x11);

	// 0xDF : RST 18h
	opcode = 0xDF;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0018);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xE0 : LDH (a8),A8
	opcode = 0xE0;
	cpu_reset_registers();
	cpu_set_A(0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(
		opcode_dict[opcode],
		mem_get_byte(0xFF00 + opcode_dict[opcode].byte1) == 0x44);

	// 0xE1 : POP HL
	opcode = 0xE1;
	cpu_reset_registers();
	cpu_set_SP(0x0FFE);
	mem_set_byte(cpu_get_SP(), 0x21);
	mem_set_byte(cpu_get_SP() + 1, 0x43);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == 0x4321 && cpu_get_SP() == 0x1000);

	// 0xE2 : LD (C),A
	opcode = 0xE2;
	cpu_reset_registers();
	cpu_set_A(0x42);
	cpu_set_C(0x33);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      mem_get_byte(0xFF33) == 0x0042);

	// 0xE5 : PUSH HL
	opcode = 0xE5;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_HL(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x0FFE);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xE6 : AND d8
	opcode = 0xE6;
	cpu_reset_registers();
	cpu_set_A(0xFF);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x33);

	// 0xE7 : RST 20h
	opcode = 0xE7;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0020);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xE8: ADD SP,r8
	opcode = 0xE8;
	cpu_reset_registers();
	cpu_set_SP(0x1111);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x1116);

	// 0xE9 : JP (HL)
	opcode = 0xE9;
	cpu_reset_registers();
	cpu_set_HL(0x8527);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_PC() == 0x8527);

	// 0xEA : LD (a16),A
	opcode = 0xEA;
	cpu_reset_registers();
	cpu_set_A(0x44);
	cpu_test_opcode(opcode_dict[opcode]);
	u16 = opcode_dict[opcode].byte1 | opcode_dict[opcode].byte2 << 8;
	cpu_print_test_result(opcode_dict[opcode], mem_get_byte(u16) == 0x44);

	// 0xEE : XOR d8
	opcode = 0xEE;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xEF : RST 28h
	opcode = 0xEF;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0028);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xF0 : LDH A8,(a8)
	opcode = 0xF0;
	cpu_reset_registers();
	mem_set_byte(0xFF00 | opcode_dict[opcode].byte1, 0x85);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x85);

	// 0xF1 : POP AF
	opcode = 0xF1;
	cpu_reset_registers();
	cpu_set_SP(0x0FFE);
	mem_set_byte(cpu_get_SP(), 0x21);
	mem_set_byte(cpu_get_SP() + 1, 0x43);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_AF() == 0x4321 && cpu_get_SP() == 0x1000);

	// 0xF2 : LD A,(C)
	opcode = 0xF2;
	cpu_reset_registers();
	cpu_set_C(0x12);
	mem_set_byte(0xFF00 | cpu_get_C(), 0x96);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0x96);

	// 0xF5 : PUSH AF
	opcode = 0xF5;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_AF(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_SP() == 0x0FFE);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xF6 : OR d8
	opcode = 0xF6;
	cpu_reset_registers();
	cpu_set_A(0xF0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xFF);

	// 0xF7 : RST 30H
	opcode = 0xF7;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0030);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	// 0xF8 : LD HL,SP+r8
	opcode = 0xF8;
	cpu_reset_registers();
	cpu_set_SP(0x1111);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_HL() == 0x1166);

	// 0xF9 : LD SP,HL
	opcode = 0xF9;
	cpu_reset_registers();
	cpu_set_HL(0x6543);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_HL() == cpu_get_SP());

	// 0xFA : LD A,(a16)
	opcode = 0xFA;
	cpu_reset_registers();
	mem_set_byte(0x1551, 0xAB);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_A() == 0xAB);

	// 0xFE : CP d8
	opcode = 0xFE;
	cpu_reset_registers();
	cpu_set_A(0x77);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0xC0);
	cpu_reset_registers();
	cpu_set_A(0x0F);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x50);
	cpu_reset_registers();
	cpu_set_A(0xF1);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode], cpu_get_F() == 0x60);

	// 0xFF : RST 38h
	opcode = 0xFF;
	cpu_reset_registers();
	cpu_set_SP(0x1000);
	cpu_set_PC(0xD1D0);
	cpu_test_opcode(opcode_dict[opcode]);
	cpu_print_test_result(opcode_dict[opcode],
			      cpu_get_SP() == 0x0FFE && cpu_get_PC() == 0x0038);
	if (mem_get_byte(cpu_get_SP()) != 0xD0 ||
	    mem_get_byte(cpu_get_SP() + 1) != 0xD1) {
		printf("unexpected memory configuration for instruction 0x%x\n",
		       opcode);
	}

	printf("\n\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\n");
	printf("All OPCODES tests SUCCEED !!!\n");
	printf("\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\\_/\n");

	return 0;
}

typedef enum {
	FLAG_SET,
	FLAG_GET,
} cpu_flag_action;

int test_flag(cpu_flag_action action, cpu_flag_name flag, cpu_flag_value value,
	      uint8_t expected_value)
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

struct opcode_info opcode_dict_CB[] = {

	{.code = 0xCB, .name = "RLC B", .byte1 = 0x00, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RLC C", .byte1 = 0x01, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RLC D", .byte1 = 0x02, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RLC E", .byte1 = 0x03, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RLC H", .byte1 = 0x04, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RLC L", .byte1 = 0x05, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RLC (HL)", .byte1 = 0x06, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RLC A", .byte1 = 0x07, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC B", .byte1 = 0x08, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC C", .byte1 = 0x09, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC D", .byte1 = 0x0A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC E", .byte1 = 0x0B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC H", .byte1 = 0x0C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC L", .byte1 = 0x0D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC (HL)", .byte1 = 0x0E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RRC A", .byte1 = 0x0F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "RL B", .byte1 = 0x10, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RL C", .byte1 = 0x11, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RL D", .byte1 = 0x12, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RL E", .byte1 = 0x13, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RL H", .byte1 = 0x14, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RL L", .byte1 = 0x15, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RL (HL)", .byte1 = 0x16, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RL A", .byte1 = 0x17, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR B", .byte1 = 0x18, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR C", .byte1 = 0x19, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR D", .byte1 = 0x1A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR E", .byte1 = 0x1B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR H", .byte1 = 0x1C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR L", .byte1 = 0x1D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR (HL)", .byte1 = 0x1E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RR A", .byte1 = 0x1F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "SLA B", .byte1 = 0x20, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SLA C", .byte1 = 0x21, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SLA D", .byte1 = 0x22, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SLA E", .byte1 = 0x23, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SLA H", .byte1 = 0x24, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SLA L", .byte1 = 0x25, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SLA (HL)", .byte1 = 0x26, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SLA A", .byte1 = 0x27, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA B", .byte1 = 0x28, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA C", .byte1 = 0x29, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA D", .byte1 = 0x2A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA E", .byte1 = 0x2B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA H", .byte1 = 0x2C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA L", .byte1 = 0x2D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA (HL)", .byte1 = 0x2E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRA A", .byte1 = 0x2F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "SWAP B", .byte1 = 0x30, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SWAP C", .byte1 = 0x31, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SWAP D", .byte1 = 0x32, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SWAP E", .byte1 = 0x33, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SWAP H", .byte1 = 0x34, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SWAP L", .byte1 = 0x35, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SWAP (HL)", .byte1 = 0x36, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SWAP A", .byte1 = 0x37, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL B", .byte1 = 0x38, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL C", .byte1 = 0x39, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL D", .byte1 = 0x3A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL E", .byte1 = 0x3B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL H", .byte1 = 0x3C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL L", .byte1 = 0x3D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL (HL)", .byte1 = 0x3E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SRL A", .byte1 = 0x3F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "BIT 0 B", .byte1 = 0x40, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 0 C", .byte1 = 0x41, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 0 D", .byte1 = 0x42, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 0 E", .byte1 = 0x43, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 0 H", .byte1 = 0x44, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 0 L", .byte1 = 0x45, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 0 (HL)", .byte1 = 0x46, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 A", .byte1 = 0x47, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 B", .byte1 = 0x48, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 C", .byte1 = 0x49, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 D", .byte1 = 0x4A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 E", .byte1 = 0x4B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 H", .byte1 = 0x4C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 L", .byte1 = 0x4D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 (HL)", .byte1 = 0x4E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 1 A", .byte1 = 0x4F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "BIT 2 B", 	.byte1 = 0x50, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 2 C", 	.byte1 = 0x51, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 2 D", 	.byte1 = 0x52, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 2 E", 	.byte1 = 0x53, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 2 H", 	.byte1 = 0x54, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 2 L", 	.byte1 = 0x55, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 2 (HL)",.byte1 = 0x56, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 2 A", 	.byte1 = 0x57, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 B", 	.byte1 = 0x58, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 C", 	.byte1 = 0x59, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 D", 	.byte1 = 0x5A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 E", 	.byte1 = 0x5B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 H", 	.byte1 = 0x5C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 L", 	.byte1 = 0x5D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 (HL)",.byte1 = 0x5E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 3 A", 	.byte1 = 0x5F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "BIT 4 B", 	.byte1 = 0x60, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 4 C", 	.byte1 = 0x61, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 4 D", 	.byte1 = 0x62, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 4 E", 	.byte1 = 0x63, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 4 H", 	.byte1 = 0x64, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 4 L", 	.byte1 = 0x65, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 4 (HL)",.byte1 = 0x66, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 4 A", 	.byte1 = 0x67, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 B", 	.byte1 = 0x68, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 C", 	.byte1 = 0x69, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 D", 	.byte1 = 0x6A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 E", 	.byte1 = 0x6B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 H", 	.byte1 = 0x6C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 L", 	.byte1 = 0x6D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 (HL)",.byte1 = 0x6E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 5 A", 	.byte1 = 0x6F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "BIT 6 B", 	.byte1 = 0x70, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 6 C", 	.byte1 = 0x71, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 6 D", 	.byte1 = 0x72, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 6 E", 	.byte1 = 0x73, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 6 H", 	.byte1 = 0x74, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 6 L", 	.byte1 = 0x75, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 6 (HL)",.byte1 = 0x76, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 6 A", 	.byte1 = 0x77, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 B", 	.byte1 = 0x78, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 C", 	.byte1 = 0x79, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 D", 	.byte1 = 0x7A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 E", 	.byte1 = 0x7B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 H", 	.byte1 = 0x7C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 L", 	.byte1 = 0x7D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 (HL)",.byte1 = 0x7E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "BIT 7 A", 	.byte1 = 0x7F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "RES 0 B", 	.byte1 = 0x80, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 0 C", 	.byte1 = 0x81, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 0 D", 	.byte1 = 0x82, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 0 E", 	.byte1 = 0x83, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 0 H", 	.byte1 = 0x84, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 0 L", 	.byte1 = 0x85, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 0 (HL)",.byte1 = 0x86, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 A", 	.byte1 = 0x87, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 B", 	.byte1 = 0x88, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 C", 	.byte1 = 0x89, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 D", 	.byte1 = 0x8A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 E", 	.byte1 = 0x8B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 H", 	.byte1 = 0x8C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 L", 	.byte1 = 0x8D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 (HL)",.byte1 = 0x8E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 1 A", 	.byte1 = 0x8F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "RES 2 B", 	.byte1 = 0x90, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 2 C", 	.byte1 = 0x91, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 2 D", 	.byte1 = 0x92, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 2 E", 	.byte1 = 0x93, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 2 H", 	.byte1 = 0x94, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 2 L", 	.byte1 = 0x95, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 2 (HL)",.byte1 = 0x96, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 2 A", 	.byte1 = 0x97, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 B", 	.byte1 = 0x98, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 C", 	.byte1 = 0x99, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 D", 	.byte1 = 0x9A, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 E", 	.byte1 = 0x9B, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 H", 	.byte1 = 0x9C, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 L", 	.byte1 = 0x9D, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 (HL)",.byte1 = 0x9E, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 3 A", 	.byte1 = 0x9F, .byte2 = 0x00 },

	{.code = 0xCB, .name = "RES 4 B", 	.byte1 = 0xA0, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 4 C", 	.byte1 = 0xA1, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 4 D", 	.byte1 = 0xA2, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 4 E", 	.byte1 = 0xA3, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 4 H", 	.byte1 = 0xA4, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 4 L", 	.byte1 = 0xA5, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 4 (HL)",.byte1 = 0xA6, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 4 A", 	.byte1 = 0xA7, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 B", 	.byte1 = 0xA8, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 C", 	.byte1 = 0xA9, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 D", 	.byte1 = 0xAA, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 E", 	.byte1 = 0xAB, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 H", 	.byte1 = 0xAC, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 L", 	.byte1 = 0xAD, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 (HL)",.byte1 = 0xAE, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 5 A", 	.byte1 = 0xAF, .byte2 = 0x00 },

	{.code = 0xCB, .name = "RES 6 B", 	.byte1 = 0xB0, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 6 C", 	.byte1 = 0xB1, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 6 D", 	.byte1 = 0xB2, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 6 E", 	.byte1 = 0xB3, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 6 H", 	.byte1 = 0xB4, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 6 L", 	.byte1 = 0xB5, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 6 (HL)",.byte1 = 0xB6, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 6 A", 	.byte1 = 0xB7, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 B", 	.byte1 = 0xB8, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 C", 	.byte1 = 0xB9, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 D", 	.byte1 = 0xBA, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 E", 	.byte1 = 0xBB, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 H", 	.byte1 = 0xBC, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 L", 	.byte1 = 0xBD, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 (HL)",.byte1 = 0xBE, .byte2 = 0x00 },
	{.code = 0xCB, .name = "RES 7 A", 	.byte1 = 0xBF, .byte2 = 0x00 },

	{.code = 0xCB, .name = "SET 0 B", 	.byte1 = 0xC0, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 0 C", 	.byte1 = 0xC1, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 0 D", 	.byte1 = 0xC2, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 0 E", 	.byte1 = 0xC3, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 0 H", 	.byte1 = 0xC4, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 0 L", 	.byte1 = 0xC5, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 0 (HL)",.byte1 = 0xC6, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 A", 	.byte1 = 0xC7, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 B", 	.byte1 = 0xC8, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 C", 	.byte1 = 0xC9, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 D", 	.byte1 = 0xCA, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 E", 	.byte1 = 0xCB, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 H", 	.byte1 = 0xCC, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 L", 	.byte1 = 0xCD, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 (HL)",.byte1 = 0xCE, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 1 A", 	.byte1 = 0xCF, .byte2 = 0x00 },

	{.code = 0xCB, .name = "SET 2 B", 	.byte1 = 0xD0, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 2 C", 	.byte1 = 0xD1, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 2 D", 	.byte1 = 0xD2, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 2 E", 	.byte1 = 0xD3, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 2 H", 	.byte1 = 0xD4, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 2 L", 	.byte1 = 0xD5, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 2 (HL)",.byte1 = 0xD6, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 2 A", 	.byte1 = 0xD7, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 B", 	.byte1 = 0xD8, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 C", 	.byte1 = 0xD9, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 D", 	.byte1 = 0xDA, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 E", 	.byte1 = 0xDB, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 H", 	.byte1 = 0xDC, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 L", 	.byte1 = 0xDD, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 (HL)",.byte1 = 0xDE, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 3 A", 	.byte1 = 0xDF, .byte2 = 0x00 },

	{.code = 0xCB, .name = "SET 4 B", 	.byte1 = 0xE0, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 4 C", 	.byte1 = 0xE1, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 4 D", 	.byte1 = 0xE2, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 4 E", 	.byte1 = 0xE3, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 4 H", 	.byte1 = 0xE4, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 4 L", 	.byte1 = 0xE5, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 4 (HL)",.byte1 = 0XE6, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 4 A", 	.byte1 = 0xE7, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 B", 	.byte1 = 0xE8, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 C", 	.byte1 = 0xE9, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 D", 	.byte1 = 0xEA, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 E", 	.byte1 = 0xEB, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 H", 	.byte1 = 0xEC, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 L", 	.byte1 = 0xED, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 (HL)",.byte1 = 0xEE, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 5 A", 	.byte1 = 0xEF, .byte2 = 0x00 },

	{.code = 0xCB, .name = "SET 6 B", 	.byte1 = 0xF0, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 6 C", 	.byte1 = 0xF1, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 6 D", 	.byte1 = 0xF2, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 6 E", 	.byte1 = 0xF3, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 6 H", 	.byte1 = 0xF4, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 6 L", 	.byte1 = 0xF5, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 6 (HL)",.byte1 = 0xF6, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 6 A", 	.byte1 = 0xF7, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 B", 	.byte1 = 0xF8, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 C", 	.byte1 = 0xF9, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 D", 	.byte1 = 0xFA, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 E", 	.byte1 = 0xFB, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 H", 	.byte1 = 0xFC, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 L", 	.byte1 = 0xFD, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 (HL)",.byte1 = 0xFE, .byte2 = 0x00 },
	{.code = 0xCB, .name = "SET 7 A", 	.byte1 = 0xFF, .byte2 = 0x00 },

};

int testsuite_opcodes_CB()
{
	printf("\n####################### CPU CB OPCODES UTEST #########################\n");

	uint8_t memfull[8000];
	uint8_t lg = 0;
	uint8_t opcode = 0x00;
	uint16_t u16;
	cpu_reset_registers();
	void cpu_reset_registers();
	memset(memfull, 0, 8000);

	// 0x00 : RLC B
	opcode = 0x00;
	cpu_reset_registers();
	cpu_set_B(0xF0);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      cpu_get_B() == 0xE1 && cpu_get_F() == 0x10);

	// 0x06 : RLC (HL)
	opcode = 0x06;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xF0);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xE1 && cpu_get_F() == 0x10);

	// 0x09 : RRC C
	opcode = 0x09;
	cpu_reset_registers();
	cpu_set_C(0xF1);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      cpu_get_C() == 0xF8 && cpu_get_F() == 0x10);

	// 0x0E : RRC (HL)
	opcode = 0x0E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xF1);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xF8 && cpu_get_F() == 0x10);

	// 0x10 : RL B
	opcode = 0x10;
	cpu_reset_registers();
	cpu_set_B(0xEF);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      cpu_get_B() == 0xDF && cpu_get_F() == 0x10);

	// 0x16 : RL (HL)
	opcode = 0x16;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xEF);	
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xDF && cpu_get_F() == 0x10);

	// 0x19 : RR C
	opcode = 0x19;
	cpu_reset_registers();
	cpu_set_C(0x7F);
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      cpu_get_C() == 0xBF && cpu_get_F() == 0x10);

	// 0x1E : RR (HL)
	opcode = 0x1E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x7F);	
	cpu_set_F(0x10);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xBF && cpu_get_F() == 0x10);

	// 0x20 : SLA B
	opcode = 0x20;
	cpu_reset_registers();
	cpu_set_B(0x88);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      cpu_get_B() == 0x10 && cpu_get_F() == 0x10);

	// 0x26 : SLA (HL)
	opcode = 0x26;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x88);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      mem_get_byte(cpu_get_HL()) == 0x10 && cpu_get_F() == 0x10);

	// 0x29 : SRA C
	opcode = 0x29;
	cpu_reset_registers();
	cpu_set_C(0x81);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      cpu_get_C() == 0xC0 && cpu_get_F() == 0x10);

	// 0x2E : SRA (HL)
	opcode = 0x2E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x81);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      mem_get_byte(cpu_get_HL()) == 0xC0 && cpu_get_F() == 0x10);

	// 0x30 : SWAP B
	opcode = 0x30;
	cpu_reset_registers();
	cpu_set_B(0xA1);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0x1A);

	// 0x36 : SWAP (HL)
	opcode = 0x36;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xA1);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(cpu_get_HL()) == 0x1A);

	// 0x39 : SRL C
	opcode = 0x39;
	cpu_reset_registers();
	cpu_set_C(0x81);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      cpu_get_C() == 0x40 && cpu_get_F() == 0x10);

	// 0x3E : SRL (HL)
	opcode = 0x3E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x81);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode],
			      mem_get_byte(cpu_get_HL()) == 0x40 && cpu_get_F() == 0x10);

	// 0x40 : BIT 0, B
	opcode = 0x40;
	cpu_reset_registers();
	cpu_set_B(0x01);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x46 : BIT 0, (HL)
	opcode = 0x46;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xF0);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x48 : BIT 1, C
	opcode = 0x49;
	cpu_reset_registers();
	cpu_set_C(0x02);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x4E : BIT 1, (HL)
	opcode = 0x4E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xF0);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x50 : BIT 2, B
	opcode = 0x50;
	cpu_reset_registers();
	cpu_set_B(0x04);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x56 : BIT 2, (HL)
	opcode = 0x56;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xF0);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x58 : BIT 3, C
	opcode = 0x59;
	cpu_reset_registers();
	cpu_set_C(0x08);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x5E : BIT 3, (HL)
	opcode = 0x5E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xF0);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x60 : BIT 4, B
	opcode = 0x60;
	cpu_reset_registers();
	cpu_set_B(0x10);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x66 : BIT 4, (HL)
	opcode = 0x66;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x0F);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x68 : BIT 5, C
	opcode = 0x69;
	cpu_reset_registers();
	cpu_set_C(0x20);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x6E : BIT 5, (HL)
	opcode = 0x6E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x0F);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x70 : BIT 6, B
	opcode = 0x70;
	cpu_reset_registers();
	cpu_set_B(0x40);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x76 : BIT 6, (HL)
	opcode = 0x76;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x0F);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x78 : BIT 7, C
	opcode = 0x79;
	cpu_reset_registers();
	cpu_set_C(0x80);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0x20);

	// 0x7E : BIT 7, (HL)
	opcode = 0x7E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x0F);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_F() == 0xA0);

	// 0x80 : RES 0, B
	opcode = 0x80;
	cpu_reset_registers();
	cpu_set_B(0xFF);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0xFE);

	// 0x86 : RES 0, (HL)
	opcode = 0x86;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xFF);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0xFE);

	// 0x88 : RES 1, B
	opcode = 0x88;
	cpu_reset_registers();
	cpu_set_B(0xFF);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0xFD);

	// 0x8E : RES 1, (HL)
	opcode = 0x8E;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xFF);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0xFD);

	// 0xB0 : RES 6, B
	opcode = 0xB0;
	cpu_reset_registers();
	cpu_set_B(0xFF);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0xBF);

	// 0xB6 : RES 6, (HL)
	opcode = 0xB6;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xFF);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0xBF);

	// 0xB8 : RES 7, B
	opcode = 0xB8;
	cpu_reset_registers();
	cpu_set_B(0xFF);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0x7F);

	// 0xBE : RES 7, (HL)
	opcode = 0xBE;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0xFF);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0x7F);

	// 0xC0 : SET 0, B
	opcode = 0xC0;
	cpu_reset_registers();
	cpu_set_B(0x00);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0x01);

	// 0xC6 : SET 0, (HL)
	opcode = 0xC6;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x00);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0x01);

	// 0xC8 : SET 1, B
	opcode = 0xC8;
	cpu_reset_registers();
	cpu_set_B(0x00);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0x02);

	// 0xCE : SET 1, (HL)
	opcode = 0xCE;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x00);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0x02);

	// 0xF0 : SET 6, B
	opcode = 0xF0;
	cpu_reset_registers();
	cpu_set_B(0x00);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0x40);

	// 0xF6 : SET 6, (HL)
	opcode = 0xF6;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x00);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0x40);

	// 0xF8 : SET 7, B
	opcode = 0xF8;
	cpu_reset_registers();
	cpu_set_B(0x00);
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], cpu_get_B() == 0x80);

	// 0xFE : SET 7, (HL)
	opcode = 0xFE;
	cpu_reset_registers();
	cpu_set_HL(0x4157);
	mem_set_byte(cpu_get_HL(), 0x00);	
	cpu_test_opcode(opcode_dict_CB[opcode]);
	cpu_print_test_result(opcode_dict_CB[opcode], mem_get_byte(0x4157) == 0x80);

	return 0;
}

int main()
{
	int ret = 0;

	ret = testsuite_flags();

	ret = testsuite_opcodes();
	ret = testsuite_opcodes_CB();

	return ret;
}