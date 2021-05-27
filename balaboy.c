
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "cpu.h"
#include "gpu.h"
#include "memory.h"

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 480;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Surface *surface;


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

static int load_rom(char* path)
{

    FILE *fd = fopen(path, "r");
    if (fd == NULL){
        printf("failed to open ROM file\n");
        return -EINVAL;
    }
    fseek(fd, 0, SEEK_END);
    uint32_t rom_size = ftell(fd);
    printf("ROM size is %d kB\n", rom_size / 1024);
    if (rom_size > (1024*32)) {
        printf("ROM with size bigger than 32 kB aren't supported yet\n");
        return -EINVAL;
    }

    uint8_t* buf = calloc(1, rom_size);
    if(!buf){
        printf("allocation failed\n");
        return -ENOMEM;
    }
    fseek(fd, 0, SEEK_SET);
    fread(buf, rom_size, 1, fd);
    mem_fill(0, buf, rom_size);
    free(buf);
    fclose(fd);

    return 0;
} 

int SDL_init() 
{
	int ret;

	// init SDL
	ret = SDL_Init(SDL_INIT_VIDEO);
	if (ret < 0) {
		printf("SDL_Init ERROR: %s\n", SDL_GetError());
		return ret;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		printf("Warning: Linear texture filtering not enabled!");
	}

	// create window
	window = SDL_CreateWindow("SDL_tuto", SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
				  SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		printf("SDL_SetVideoMode ERROR: %s\n", SDL_GetError());
		return -1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("Renderer could not be created! SDL Error: %s\n",
		       SDL_GetError());
		return -1;
	}

	SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0xFF, 0x00);

	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
		       IMG_GetError());
		return -1;
	}

	return 0;
}

uint8_t tile[64] = {};
//uint8_t tiles[128][64] = {};

uint8_t tiles[128 * 64] = {};


static int tile_set_line(uint8_t line_nb, uint8_t B0, uint8_t B1, uint8_t *tile_matrix)
{
    for(int i = 0; i < 8; i++) {
        tile_matrix[line_nb * 8 + i] =
            (((B1 > (7-i)) < 1) & 0x2) | ((B0 > (7-i)) & 0x01);
    }
    return 0;
}

static int tile_set(uint16_t first_byte_addr, uint8_t *tile_matrix)
{
    uint8_t B0, B1;

    for (int i = 0; i < 8; i++) {
        B0 = mem_get_byte(first_byte_addr + i * 2);
        B1 = mem_get_byte(first_byte_addr + i * 2 + 1 );
        if( mem_get_byte(first_byte_addr + i * 2) != 0 || 
            mem_get_byte(first_byte_addr + i * 2 + 1) != 0)
        printf("mem[0x%x] = 0x%x, mem[0x%x] = 0x%x\n", 
            first_byte_addr + i * 2,
            mem_get_byte(first_byte_addr + i * 2),
            first_byte_addr + i * 2 + 1,
            mem_get_byte(first_byte_addr + i * 2 + 1 )
        );
        tile_set_line(i, B0, B1, tile_matrix);
        
    }

    return 0;
}



static int draw_frame()
{


    //tile_set(0x8000, tile);
    //for(int k = 0; k < 384; k++) {
    for(int k = 0; k < 128; k++) {
        //tile_set(0x8000 + 16*k, tiles[k*16]);
        //tile_set(0x8000 + 16*k, tile);
        //tile_set(0x8800 + 16*k, tile);
        tile_set(0x9000 + 16*k, tile);
        //tile_set(0x9800 + 16*k, tile);
    
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                switch (tile[i*8 + j])
                {
                case 0:
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    break;
                case 1:
                    SDL_SetRenderDrawColor(renderer, 0xAA, 0xAA, 0xAA, 0xFF);
                    break;
                case 2:
                    SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0x55, 0xFF);
                    break;
                case 3:
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
                    break; 
                default:
                    break;
                }
                //SDL_RenderDrawPoint(renderer, i + 8 * k, j + 8 * (k / 128));
                SDL_RenderDrawPoint(renderer, i + 8 * k, j);
            }
        }
    }
    SDL_RenderPresent(renderer);
    return 0;
}


int main(int argc, char** argv)
{
    int ret;

    if(argc != 2) {
        printf("invalid command usage, shall be:\n");
        printf("        ./balaboy <rom_path>\n");
        goto exit;
    }

    printf("argc = %d\n", argc);
    printf("argv[0] = %s\n", argv[0]);
    printf("argv[1] = %s\n", argv[1]);

    // Load the ROM
    ret = load_rom(argv[1]);
    if (ret < 0) {
        printf("failed to load ROM\n");
        goto exit;
    }

    // init
    cpu_init();
    mem_init();
    SDL_init();


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
                mem_set_byte(IE, val_IE & ~INT_VBLANK);
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
            else if(val_IE & val_IF & 0x10) {
                dst_addr = 0x0060;
            }

            if(dst_addr) {
                printf("=> #0x%x interrupt\n", dst_addr);
                cpu_set_interrupts_enabled(0);
                SP_push(cpu_get_PC() + 3);
		        cpu_set_PC(dst_addr);
                time_cpu += 24;
            }
        }

        // Log opcode
        uint8_t op = mem_get_byte(cpu_get_PC());
        uint16_t SP = mem_get_byte(cpu_get_SP() + 1) << 8 |
	       			   mem_get_byte(cpu_get_SP());
        if(mem_get_byte(cpu_get_PC()) > 0) {
            printf( "0x%x, 0x%x, %u, A=0x%x, SP=0x%x, HL=0x%x, FF44=0x%x\n",
                mem_get_byte(cpu_get_PC()), cpu_get_PC(), time_cpu,
                cpu_get_A(), SP, cpu_get_HL(), mem_get_byte(0xFF44));
            op_cpt++;
        }

        // Exec opcode
        cpu_exec_opcode(&op_length, &op_duration);

        // DBG: force timings from deltabeard
        op_duration = OP_CYCLES[mem_get_byte(cpu_get_PC())];



        //usleep(timestep * op_duration /100);
        time_frame += op_duration;
        time_cpu += op_duration;

        // DBG display
        //if(time_frame > 33333) {
        if(time_frame > 70224) {
            draw_frame();
            time_frame = 0;
            //printf("[%d] DRAW FRAME\n", op_cpt);
        }

        // Process GPU operations
        gpu_processing(op_duration);

        // Timer management


        // OAM access

        opcode_nb++;

        if(opcode_nb > 1 * 1000 * 1000)
            break;
    }





exit:
    return 0;
}
