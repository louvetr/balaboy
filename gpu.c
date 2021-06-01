#include "cpu.h"
#include "gpu.h"
#include "memory.h"

#define DURATION_HBLANK 204
#define DURATION_VBLANK 4560
#define DURATION_OAM 80
#define DURATION_LCD 172
#define DURATION_LINE 456

static uint8_t gpu_line = 0;

const int SCREEN_WIDTH_VRAM = 1024;
const int SCREEN_HEIGHT_VRAM = 32;

const int SCREEN_WIDTH = 256 /*160*/;
const int SCREEN_HEIGHT = 256 /*144*/;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Surface *surface;


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
	uint8_t p0, p1;

    for(int i = 0; i < 8; i++) {
		p0 = (B0 >> (7-i)) & 0x01;
		p1 = ((B1 >> (7-i)) << 1) & 0x2;
        tile_matrix[line_nb * 8 + i] = p1 | p0;
    }
    return 0;
}

static int tile_set_line_in_background(uint8_t line_nb, uint8_t B0, uint8_t B1, uint8_t *addr)
{
	uint8_t p0, p1;

    for(int i = 0; i < 8; i++) {
		p0 = (B0 >> (7-i)) & 0x01;
		p1 = ((B1 >> (7-i)) << 1) & 0x2;
        *(addr + i) = p1 | p0;
    }
    return 0;
}

static int tile_set(uint16_t first_byte_addr, uint8_t *tile_matrix)
{
    uint8_t B0, B1;

    for (int i = 0; i < 8; i++) {
        B0 = mem_get_byte(first_byte_addr + i * 2);
        B1 = mem_get_byte(first_byte_addr + i * 2 + 1 );
        /*if( mem_get_byte(first_byte_addr + i * 2) != 0 || 
            mem_get_byte(first_byte_addr + i * 2 + 1) != 0)
        	printf("mem[0x%x] = 0x%x, mem[0x%x] = 0x%x\n", 
            	first_byte_addr + i * 2,
            	mem_get_byte(first_byte_addr + i * 2),
            	first_byte_addr + i * 2 + 1,
            	mem_get_byte(first_byte_addr + i * 2 + 1 )
        	);*/
        tile_set_line(i, B0, B1, tile_matrix);
        
    }

    return 0;
}



static int draw_frame_VRAM()
{
	static uint32_t cpt_w = 0;
	static uint32_t cpt_lg = 0;
	static uint32_t cpt_dg = 0;
	static uint32_t cpt_d = 0;

    //tile_set(0x8000, tile);
    for(int l = 0; l < 4; l++) {
        for(int k = 0; k < 128; k++) {
            //tile_set(0x8000 + 16*k, tiles[k*16]);
            tile_set(0x8000 + 0x800*l + 16*k, tile);
            //tile_set(0x8000 + 16*k, tile);
            //tile_set(0x8800 + 16*k, tile);
            //tile_set(0x9000 + 16*k, tile);
            //tile_set(0x9800 + 16*k, tile);
        
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    switch (tile[i + j*8])
                    {
                    case 0:
                        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
						cpt_w++;
                        break;
                    case 1:
                        SDL_SetRenderDrawColor(renderer, 0xAA, 0xAA, 0xAA, 0xFF);
						cpt_lg++;
                        break;
                    case 2:
                        SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0x55, 0xFF);
						cpt_dg++;
                        break;
                    case 3:
                        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
						cpt_d++;
                        break; 
                    default:
                        break;
                    }
                    SDL_RenderDrawPoint(renderer, i + 8 * k, j + l*8);
                }
            }
        }
	}
    SDL_RenderPresent(renderer);

	//printf("[pixel_distribution][w,lg,dg,d] %u, %u, %u, %u\n", cpt_w, cpt_lg, cpt_dg, cpt_d);

    return 0;
}


static int draw_frame_SCREEN()
{
	uint8_t background[256*256 + 1] = {};
	uint16_t tile_map_addr;
	uint16_t tile_data_addr;
	uint8_t lcdc = mem_get_byte(LCDC);
	lcdc = 0xd3;

	if(lcdc & 0x08)
		tile_map_addr = 0x9C00;
	else
		tile_map_addr = 0x9800;

	if(lcdc & 0x10)
		tile_data_addr = 0x8000;
	else
		tile_data_addr = 0x8800;

	int idx_max = 0;
	tile_map_addr = 0x9800;

	// go through the 32x32 tile map
	for (int i = 0; i < 32; i++){
		for (int j = 0; j < 32; j++){
			uint8_t tile_idx = mem_get_byte(tile_map_addr + i*32 + j);
			//printf("%-3d,", tile_idx);

			// fill the background buffer with the current tile line by line
			for (int k = 0; k < 8; k++) {
    		    uint8_t B0 = mem_get_byte(tile_data_addr + (tile_idx + k) * 2);
    		    uint8_t B1 = mem_get_byte(tile_data_addr + (tile_idx + k) * 2 + 1 );
				int idx = i*32*64 + k*256 + j*8;
				//printf("%d,", idx);
    		    tile_set_line_in_background(i, B0, B1, &background[idx]);
				if(idx > idx_max)
					idx_max = idx;
			}
		}			
		//printf("\n");
	}
	//printf("\n");

	//printf("max_idx =%d (%d)\n", idx_max, 256*256);

	// DBG: display BG ///////////////////////////////////////
	int x, y;
	for(y = 0; y < 256; y++) {
        for(x = 0; x < 256; x++) {
            switch (background[y*256 + x])
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
            SDL_RenderDrawPoint(renderer, x, y);
        }
	}
    SDL_RenderPresent(renderer);

	return 0;
	// DBG //////////////////////////////////////////////////////////////////

#if 0
	/*printf("\n");
	printf("\n");
	for (int i = 0; i < 256; i++){
		for (int j = 0; j < 256; j++) {
			printf("%d", background[i*256+j]);
		}
		printf("\n");
	}*/

	// fill and render screen
	uint16_t scx = mem_get_byte(SCX);
	uint16_t scy = mem_get_byte(SCY);
	/*uint16_t max_x = scx + 160;
	uint16_t max_y = scy + 144;*/
	
	//int x, y;
    /*for(y = scy; y < max_y; y++) {
        for(x = scx; x < max_x; x++) {*/
    for(y = 0; y < 144; y++) {
        for(x = 0; x < 160; x++) {
            switch (background[(scy + y) * 256 + scx + x])
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
            SDL_RenderDrawPoint(renderer, x, y);
        }
	}
    SDL_RenderPresent(renderer);

	static int nb_frame = 0;
	printf("[%d] scx=%d, scy=%d, map=0x%x, data=%x, LCDC=0x%x, map0=%d\n",
			nb_frame, scx, scy, tile_map_addr, tile_data_addr, mem_get_byte(LCDC), mem_get_byte(tile_map_addr));
	nb_frame++;

	return 0;
#endif
}


static int cptt;

int gpu_processing(uint8_t op_duration)
{
	static uint8_t mode = 2;
	static uint32_t time_gpu = 4;

	time_gpu += op_duration;

	/*printf("[gpu_loop] time_gpu = %d, mode = %d, linr_prev = %d, line = %d\n",
	       time_gpu, mode, line_prev, gpu_line);*/

	switch (mode) {
	case HBLANK:
		if (time_gpu > DURATION_HBLANK) {
			time_gpu -= DURATION_HBLANK;
			gpu_line++;
			//printf("[%d] line set to %d\n", __LINE__, gpu_line);
			mem_set_byte(LY, gpu_line);
			if (gpu_line >= 144) {
				mode = VBLANK;
				// Trigger VBLANK interrupt
				printf("=> trigger VBLANK interrupt #%d\n", cptt);
				cptt++;
				uint8_t val = mem_get_byte(IF);
	           	mem_set_byte(IF, val | INT_VBLANK);

				// dbg functions        
				//draw_frame_VRAM();
				draw_frame_SCREEN();
				dump_VRAM();
			}
			else {
				mode = OAM_ACCESS;
			}
		}
		break;

	case VBLANK:
		if (time_gpu > DURATION_LINE) {
			time_gpu -= DURATION_LINE;

			//printf("[%d] line set to %d\n", __LINE__, gpu_line);

			if (gpu_line >= 153) {
				mode = OAM_ACCESS;
				gpu_line = 0;

				//printf("[%d] line set to %d\n", __LINE__, gpu_line);
				// TODO: move SDL rendering in gpu.c
				//gpu_render_frame();
			}
			else {
				gpu_line++;
			}

			mem_set_byte(LY, gpu_line);


		}
		break;

	case OAM_ACCESS:
		if (time_gpu > DURATION_OAM) {
			time_gpu -= DURATION_OAM;
			mode = LCD_DRAWING;
		}
		break;

	case LCD_DRAWING:
		if (time_gpu > DURATION_LCD) {
			time_gpu -= DURATION_LCD;
			mode = HBLANK;
		}

		break;

	default:
		printf("[%s] ERROR: invalid mode, should not happen\n",
		       __func__);
	}

}

