
#include "memory.h"

// declare locally the memory array
static uint8_t memory[MEMORY_SIZE];

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


uint8_t mem_get_byte(uint16_t addr) {

    if(addr == 0xFF00) { // P1
        // TODO: manage inputs
        return memory[addr] | 0xC0 | 0x0F;
    }

    return memory[addr];
}

void mem_set_byte(uint16_t addr, uint8_t value)
{
    // Do no overwrite cartridge ROM
    if (addr < 0x8000)
        return;

    switch(addr) {

    case 0xDFE9: // WRAM
		printf("force_log --- wrire into 0xDFE9\n");
		//force_log = 1;
        memory[addr] = value;
        break;       

    case 0xFFA6: // WRAM
		printf("force_log --- wrire into 0xFFA6\n");
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

    if(addr == 0x2000)
        printf("tototototo\n");
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
