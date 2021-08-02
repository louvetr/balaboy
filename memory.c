
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "memory.h"

// declare locally the memory array
static uint8_t memory[MEMORY_SIZE];

static struct cartridge_info cart;

// debug function
int dump_VRAM()
{
	static int i = 0;
	i++;

	if (i % 1000 != 0)
		return 0;

	FILE *fd;
	char filename[64];
	snprintf(filename, 64, "/tmp/dump_vram_balaboy_%d", i);
	fd = fopen(filename, "w");

	fwrite(memory + 0x8000, 0x2000, 1, fd);
	fclose(fd);

	return 0;
}

static void mem_OAM_copy(uint8_t start_addr)
{
	uint16_t src = start_addr << 8;
	memcpy(&memory[0xFE00], &memory[src], 0xA0);
	//TODO: wait 160 usec
}

uint8_t mem_get_byte(uint16_t addr)
{
	if (cart.type == TYPE_MBC1 || cart.type == TYPE_MBC1_RAM ||
	    cart.type == TYPE_MBC1_RAM_BATT) {
		uint16_t offset;
		uint16_t idx;
		if (addr >= 0x4000 && addr < 0x7fff) {
			offset = BANK_SIZE_ROM * cart.ROM_bank_active;
			idx = addr - 0x4000;
			return cart.mem[offset + idx];
		}
		if (addr >= 0xA000 && addr < 0xBfff &&
		    cart.RAM_banking_enable) {
			offset = BANK_SIZE_RAM * cart.RAM_bank_active;
			idx = addr - 0xA000;
			return cart.mem[offset + idx];
		}
	}

	if (addr == 0xFF00) { // P1
		return 0xC0 | (memory[addr] & 0x3F);
	}

	return memory[addr];
}

void mem_set_byte(uint16_t addr, uint8_t value)
{
	uint8_t tmp;

	// Do no overwrite cartridge ROM
	if (addr < 0x8000)
		return;

	if (cart.type == TYPE_MBC1 || cart.type == TYPE_MBC1_RAM ||
	    cart.type == TYPE_MBC1_RAM_BATT) {
		uint16_t offset;
		uint16_t idx;
		if (addr >= 0xA000 && addr < 0xBfff &&
		    cart.RAM_banking_enable) {
			offset = BANK_SIZE_RAM * cart.RAM_bank_active;
			idx = addr - 0xA000;
			cart.mem[offset + idx] = value;
			return;
		}

		if (addr >= 0x6000 && addr < 0x7fff) {
			if (value && 0xFE)
				cart.mode = MODE_MBC1_4_32;
			else
				cart.mode = MODE_MBC1_16_8;
			return;
		}

		if (addr >= 0x4000 && addr < 0x5fff) {
			switch (cart.mode) {
			case MODE_MBC1_16_8:
				cart.ROM_bank_active =
					(cart.ROM_bank_active & 0x1f) |
					((value & 0x03) << 5);
				break;
			case MODE_MBC1_4_32:
				cart.RAM_bank_active = value & 0x02;
				break;
			default:
				printf("Invalid MBC1 mode while writing to [0x4000-0x5fff]\n");
				exit(0);
			}
			return;
		}

		if (addr >= 0x2000 && addr < 0x2fff) {
			uint8_t fixed_value;
			// fix the unconsistant ROM bank mapping from original hardware
			switch (value) {
			case 0x00:
			case 0x20:
			case 0x40:
			case 0x60:
				fixed_value = value + 1;
				break;
			default:
				fixed_value = value;
			}
			cart.ROM_bank_active = (cart.ROM_bank_active & 0x60) |
					       (fixed_value & 0x1f);
			return;
		}

		if (addr <= 0x1fff) {
			if (value == 0x0A)
				cart.RAM_banking_enable = 1;
			else if (value == 0x00)
				cart.RAM_banking_enable = 0;
			return;
		}
	}

	switch (addr) {
	case 0xDFE9: // WRAM
		memory[addr] = value;
		break;

	case 0xFF00: // P1 input
		tmp = memory[addr];
		memory[addr] = (value & 0xF0) | (input_get() & 0x0F);
		break;

	case 0xFFA6: // WRAM
		memory[addr] = value;
		break;

	case 0xFF04: // DIV
		memory[addr] = 0x0;
		break;

	case 0xFF46: // DMA
		memory[addr] = value;
		mem_OAM_copy(value);
		break;

	default:
		memory[addr] = value;

		//if(addr >= 0x8000 && addr <= 0x9FFF) {
		/*if(addr >= 0x9800 && addr < 0x9C00) {
            printf("tilemap[0x%x] = 0x%x\n", addr, memory[addr]);
            //set_force_log();
        }*/
	}
}

void mem_fill(uint16_t addr, uint8_t *data, uint16_t size)
{
	memcpy(memory + addr, data, size);
}

void mem_DIV_increment(uint8_t opcode_duration)
{
	memory[DIV] += opcode_duration / 4;
}

void mem_init()
{
	memory[0xFF05] = 0x00;
	memory[0xFF06] = 0x00;
	memory[0xFF07] = 0x00;
	memory[0xFF10] = 0x80;
	memory[0xFF11] = 0xBF;
	memory[0xFF12] = 0xF3;
	memory[0xFF14] = 0xBF;
	memory[0xFF16] = 0x3F;
	memory[0xFF17] = 0x00;
	memory[0xFF19] = 0xBF;
	memory[0xFF1A] = 0x7F;
	memory[0xFF1B] = 0xFF;
	memory[0xFF1C] = 0x9F;
	memory[0xFF1E] = 0xBF;
	memory[0xFF20] = 0xFF;
	memory[0xFF21] = 0x00;
	memory[0xFF22] = 0x00;
	memory[0xFF23] = 0xBF;
	memory[0xFF24] = 0x77;
	memory[0xFF25] = 0xF3;
	memory[0xFF26] = 0xF1;
	memory[0xFF40] = 0x91;
	memory[0xFF42] = 0x00;
	memory[0xFF43] = 0x00;
	memory[0xFF45] = 0x00;
	memory[0xFF47] = 0xFC;
	memory[0xFF48] = 0xFF;
	memory[0xFF49] = 0xFF;
	memory[0xFF4A] = 0x00;
	memory[0xFF4B] = 0x00;
	memory[0xFFFF] = 0x00;
}

int mem_load_rom(char *path)
{
	FILE *fd = fopen(path, "r");
	if (fd == NULL) {
		printf("failed to open ROM file\n");
		return -EINVAL;
	}
	fseek(fd, 0, SEEK_END);
	uint32_t rom_size = ftell(fd);
	//printf("ROM size is %d kB\n", rom_size / 1024);
	/*if (rom_size > (1024 * 32)) {
		printf("ROM with size bigger than 32 kB aren't supported yet\n");
		return -EINVAL;
	}*/

	//uint8_t* buf = calloc(1, rom_size);
	cart.mem = calloc(1, rom_size);
	if (!cart.mem) {
		printf("allocation failed\n");
		return -ENOMEM;
	}
	fseek(fd, 0, SEEK_SET);
	fread(cart.mem, rom_size, 1, fd);
	mem_fill(0, cart.mem, rom_size);
	//free(buf);
	fclose(fd);


	cart.RAM_banking_enable = 0;
	// Check the cartridge type and act accordingly
	printf("Cartridge type = %d\n", cart.mem[0x147]);
	cart.type = cart.mem[0x147];
	switch (cart.mem[0x147]) {
	case TYPE_MBC0:
		break;
	case TYPE_MBC1:
	case TYPE_MBC1_RAM:
	case TYPE_MBC1_RAM_BATT:
		break;
	default:
		printf("Cartridge type #%d not supported\n", cart.mem[0x147]);
		exit(0);
	}

	switch (cart.mem[0x148]) {
	case 0:
		cart.ROM_bank_nb = 2;
		break;
	case 1:
		cart.ROM_bank_nb = 4;
		break;
	case 2:
		cart.ROM_bank_nb = 8;
		break;
	case 3:
		cart.ROM_bank_nb = 16;
		break;
	case 4:
		cart.ROM_bank_nb = 32;
		break;
	case 5:
		cart.ROM_bank_nb = 64;
		break;
	case 6:
		cart.ROM_bank_nb = 128;
		break;
	case 52:
		cart.ROM_bank_nb = 72;
		break;
	case 53:
		cart.ROM_bank_nb = 80;
		break;
	case 54:
		cart.ROM_bank_nb = 96;
		break;
	default:
		printf("Cartridge invalid ROM bank number\n");
		exit(0);
	}
	printf("Cartridge ROM bank number = %d\n", cart.ROM_bank_nb);

	switch (cart.mem[0x149]) {
	case 0:
	case 1:
	case 2:
		cart.RAM_bank_nb = 1;
		break;
	case 4:
		cart.RAM_bank_nb = 16;
		break;
	default:
		printf("Cartridge invalid RAM bank number\n");
		exit(0);
	}
	printf("Cartridge RAM bank number = %d\n", cart.RAM_bank_nb);



	// TODO: write a exit function which will free cart and other ressources

	return 0;
}