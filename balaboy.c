
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "input.h"
#include "gpu.h"
#include "memory.h"
#include "time.h"

static int force_log = 0;



uint8_t OP_CYCLES[0x100] = {
	//   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4,    // 0x00
	4,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,    // 0x10
	8,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,    // 0x20
	8,12, 8, 8,12,12,12, 4, 8, 8, 8, 8, 4, 4, 8, 4,    // 0x30
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x40
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x50
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x60
	8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x70
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x80
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x90
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0xA0
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0xB0
	8,12,12,12,12,16, 8,32, 8, 8,12, 8,12,12, 8,32,    // 0xC0
	8,12,12, 0,12,16, 8,32, 8, 8,12, 0,12, 0, 8,32,    // 0xD0
	12,12, 8, 0, 0,16, 8,32,16, 4,16, 0, 0, 0, 8,32,    // 0xE0
	12,12, 8, 4, 0,16, 8,32,12, 8,16, 4, 0, 0, 8,32     // 0xF0
};



void set_force_log()
{
    force_log = 1;   
}

int main(int argc, char** argv)
{
    int ret;

    if(argc < 2 || argc > 3) {
        printf("Invalid command usage, shall be:\n");
        printf("        ./balaboy <rom_path> <screen_scale>\n");
        printf("Example:\n");
        printf("        ./balaboy tetris.gb 3\n");
        goto exit;
    }

    /*printf("argc = %d\n", argc);
    printf("argv[0] = %s\n", argv[0]);
    printf("argv[1] = %s\n", argv[1]);*/

    // Load the ROM
    ret = mem_load_rom(argv[1]);
    if (ret < 0) {
        printf("failed to load ROM\n");
        goto exit;
    }

    if(argc >= 3) {
        int screen_scale = atoi(argv[2]);
        if (screen_scale > 6 || screen_scale <= 0) {
            printf("Invalid screen_scale value. It must be contained whitin [1,6]\n");
            goto exit;
        }
        gpu_set_scale(screen_scale);
    }

    // init
    SDL_init();
    cpu_init();
    input_init();
    mem_init();
    time_init();


    uint8_t op_length, op_duration;
    uint32_t op_cpt = 0;
    // cpu loop


    uint32_t time_frame = 4;
    uint32_t time_cpu = 4;
    uint32_t opcode_nb = 0;


    // Main loop
    while(1) {

        if(op_cpt == 12340)
            time_frame++;

        // Manage Interrupts
        if(cpu_get_interrupts_enabled() && 1)
        {
            uint8_t val_IE = mem_get_byte(IE);
            uint8_t val_IF = mem_get_byte(IF);
            uint16_t dst_addr = 0x00;

            // VBLANK
            if(val_IE & val_IF & INT_VBLANK) {
                dst_addr = INT_VBLANK_ADDR;
                mem_set_byte(IF, val_IF & ~INT_VBLANK);
            }
            // LCDC
            else if(val_IE & val_IF & 0x2) {
                dst_addr = 0x0048;

            }
            // TIMER
            else if(val_IE & val_IF & 0x4) {
                dst_addr = 0x0050;

            }
            // Serial I/O 
            else if(val_IE & val_IF & 0x8) {
                //TODO
                dst_addr = 0x0058;
            }
            // Joypad Input
            else if(val_IE & val_IF & INT_JOYPAD) {
                dst_addr = 0x0060;
                mem_set_byte(IF, val_IF & ~INT_JOYPAD);
            }

            if(dst_addr) {                
                if(dst_addr != 0x58 && dst_addr == 0x0060)
                    printf("=> #0x%x interrupt\n", dst_addr);
                cpu_set_interrupts_enabled(0);
                SP_push(cpu_get_PC()/* + 3*/);
		        cpu_set_PC(dst_addr);
                time_cpu += 24;
            }
        }

        // Log opcode
        uint8_t op = mem_get_byte(cpu_get_PC());
        uint16_t SP = mem_get_byte(cpu_get_SP() + 1) << 8 |
	       			   mem_get_byte(cpu_get_SP());
        /*if(0 && mem_get_byte(cpu_get_PC()) > 0 && 
            (time_cpu > 190000568 || (time_cpu < 37000000))
            ) {*/
            /*printf( "0x%x, 0x%x, %u, A=0x%x, SP=0x%x, HL[0x%x]=0x%x, FF44=0x%x, FF40=0x%x Z=%d,N=%d,H=%d,C=%d\n",
                mem_get_byte(cpu_get_PC()), cpu_get_PC(), time_cpu,
                cpu_get_A(), SP, cpu_get_HL(), mem_get_byte(cpu_get_HL()),
                mem_get_byte(0xFF44), mem_get_byte(0xFF40),
                cpu_get_flag(FLAG_ZERO), cpu_get_flag(FLAG_SUB), cpu_get_flag(FLAG_HALF_CARRY), cpu_get_flag(FLAG_CARRY));*/
            op_cpt++;
        //}

        // Exec opcode
        cpu_exec_opcode(&op_length, &op_duration);

        // DBG: force timings from deltabeard
        //op_duration = OP_CYCLES[mem_get_byte(cpu_get_PC())];



        //usleep(timestep * op_duration /100);
        time_frame += op_duration;
        time_cpu += op_duration;

        // Process GPU operations
        gpu_processing(op_duration);

        // Timers management
        mem_DIV_increment(op_duration);


        // Input management
        input_scan();


        if(force_log && 0) {
            printf( "[after] 0x%x, 0x%x, %u, A=0x%x, SP=0x%x, HL=0x%x, (HL)=0x%x, FFA6=0x%x, FF00=0x%x, FFF0=0x%x, Z=%d,N=%d,H=%d,C=%d\n",
                mem_get_byte(cpu_get_PC()), cpu_get_PC(), time_cpu,
                cpu_get_A(), SP, cpu_get_HL(), mem_get_byte(cpu_get_HL()),
                mem_get_byte(0xFFA6), mem_get_byte(0xFF00), mem_get_byte(0xFFF0),
                cpu_get_flag(FLAG_ZERO), cpu_get_flag(FLAG_SUB), cpu_get_flag(FLAG_HALF_CARRY), cpu_get_flag(FLAG_CARRY));
            force_log = 0;
        }


        opcode_nb++;

    }



exit:
    return 0;
}
