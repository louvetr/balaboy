
#include "memory.h"

// declare locally the memory array
static uint8_t memory[MEMORY_SIZE];

int dump_VRAM()
{
	static int i = 0;
    i++;

	if (i % 100 != 0)
		return 0;

	FILE *fd;
	char filename[64];
	snprintf(filename, 64, "/tmp/dump_vram_balaboy_%d", i);
	fd = fopen(filename, "w");

	fwrite(memory + 0x8000, 0x2000, 1, fd);
	fclose(fd);
    //i++;

	return 0;
}


static void mem_OAM_copy(uint8_t start_addr)
{
    uint16_t src = start_addr << 8;
    memcpy(&memory[src], &memory[0xFE00], 0xA0);
    //TODO: wait 160 usec
}


uint8_t mem_get_byte(uint16_t addr) {return memory[addr]; }

void mem_set_byte(uint16_t addr, uint8_t value)
{
    switch(addr) {

    case 0xFF04: // DIV
        memory[addr] = 0x0;
        break;       

    case 0xFF46: // DMA
        mem_OAM_copy(value);
        break;

    default:

        /*if(addr == 0xFF44 && memory[0xFF44] == 0x1A && (value == 0x02 || value == 0x01))
            printf("MINDFUCK ---------------------------------\n");*/

        memory[addr] = value;

        if(addr >= 0x8000 && addr <= 0x9FFF) {
            addr++;
            addr--;
            printf("memory[0x%x] = 0x%x\n", addr, memory[addr]);
        }

    }
}

void mem_fill(uint16_t addr, uint8_t *data, uint16_t size)
{
    memcpy(memory + addr, data, size);
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