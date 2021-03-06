#include "cpu.h"
#include "gpu.h"
#include "memory.h"
#include "time.h"

#define DURATION_HBLANK 204
#define DURATION_VBLANK 4560
#define DURATION_OAM 80
#define DURATION_LCD 172
#define DURATION_LINE 456

#define COLOR32_WHITE 0xFFFFFFFF
#define COLOR32_LIGHTGRAY 0xFFAAAAAA
#define COLOR32_DARKGRAY 0xFF555555
#define COLOR32_BLACK 0xFF000000


static uint8_t gpu_line = 0;

const int SCREEN_WIDTH_VRAM = 1024;
const int SCREEN_HEIGHT_VRAM = 32;

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

SDL_Window *window;
SDL_Texture *texture;
SDL_Surface *surface;

static uint8_t scale = 2;

void gpu_set_scale(uint8_t value)
{
	scale = value;
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
	window = SDL_CreateWindow("BalaBoy", SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * scale,
				  SCREEN_HEIGHT * scale, SDL_WINDOW_SHOWN);
	if (!window) {
		printf("SDL_SetVideoMode ERROR: %s\n", SDL_GetError());
		return -1;
	}

	//SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0xFF, 0x00);

	// get window surface
	surface = SDL_GetWindowSurface(window);
	if (!surface) {
		printf("SDL_GetWindowSurface ERROR: %s\n", SDL_GetError());
		return -1;
	}

	/*int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
		       IMG_GetError());
		return -1;
	}*/

	return 0;
}

uint8_t tile[64] = {};

// debug function to investigate VRAM
static int tile_set_line_VRAM(uint8_t line_nb, uint8_t B0, uint8_t B1,
			      uint8_t *tile_matrix)
{
	uint8_t p0, p1;

	for (int i = 0; i < 8; i++) {
		p0 = (B0 >> (7 - i)) & 0x01;
		p1 = ((B1 >> (7 - i)) << 1) & 0x2;
		tile_matrix[line_nb * 8 + i] = p1 | p0;
	}
	return 0;
}

static int tile_set_line_background(uint8_t B0, uint8_t B1, uint8_t *addr)
{
	uint8_t p0, p1, pp, pval;
	uint8_t palette = mem_get_byte(0xff47);

	// TODO : set BG palette here

	for (int i = 0; i < 8; i++) {
		p0 = (B0 >> (7 - i)) & 0x01;
		p1 = ((B1 >> (7 - i)) << 1) & 0x2;
		pp = p1 | p0;

		pp = p1 | p0;

		switch (pp) {
		case 0x00:
			pval = palette & 0x03;
			break;
		case 0x01:
			pval = (palette >> 2) & 0x03;
			break;
		case 0x02:
			pval = (palette >> 4) & 0x03;
			break;
		case 0x03:
			pval = (palette >> 6) & 0x03;
			break;
		}

		*(addr + i) = pval;
	}
	return 0;
}

static uint8_t get_sprite_pixel(uint8_t pp, uint8_t palette_idx)
{
	uint8_t pval, palette_byte;

	switch (palette_idx) {
	case 0:
		palette_byte = mem_get_byte(0xff48);
		break;
	case 1:
		palette_byte = mem_get_byte(0xff49);
		break;
	default:
		printf("[%s] invalid input palette value\n", __func__);
		exit(0);
	}

	switch (pp) {
	case 0x00:
		pval = palette_byte & 0x03;
		break;
	case 0x01:
		pval = (palette_byte >> 2) & 0x03;
		break;
	case 0x02:
		pval = (palette_byte >> 4) & 0x03;
		break;
	case 0x03:
		pval = (palette_byte >> 6) & 0x03;
		break;
	}

	return pval;
}

static int tile_set_line_sprite(uint8_t B0, uint8_t B1, uint8_t layer,
				uint8_t x_flip, uint8_t palette, uint8_t *addr)
{
	uint8_t p0, p1, pp, ppalette;

	// TODO: factorize in one loop
	if (x_flip) {
		for (int i = 7; i >= 0; i--) {
			p0 = (B0 >> (7 - i)) & 0x01;
			p1 = ((B1 >> (7 - i)) << 1) & 0x2;
			pp = p1 | p0;

			ppalette = get_sprite_pixel(pp, palette);

			// ignore blank sprite pixel since they are transparent
			if (ppalette)
				continue;

			// above BG
			if (layer == 0) {
				*(addr + i) = ppalette;
			}
			// under BG
			else {
				// print pixel only if BG pixel is WHITE (= 0xFF ?)
				if (*(addr + i) == 0x00)
					*(addr + i) = ppalette;
			}
		}
	} else {
		for (int i = 0; i < 8; i++) {
			p0 = (B0 >> (7 - i)) & 0x01;
			p1 = ((B1 >> (7 - i)) << 1) & 0x2;
			pp = p1 | p0;

			ppalette = get_sprite_pixel(pp, palette);

			// ignore blank sprite pixel since they are transparent
			if (ppalette == 0x00)
				continue;

			// above BG
			if (layer == 0) {
				*(addr + i) = ppalette;
			}
			// under BG
			else {
				// print pixel only if BG pixel is WHITE (= 0xFF ?)
				if (*(addr + i) == 0x00)
					*(addr + i) = ppalette;
			}
		}
	}

	return 0;
}

// debug function to investigate VRAM
static int tile_set_VRAM(uint16_t first_byte_addr, uint8_t *tile_matrix)
{
	uint8_t B0, B1;

	for (int i = 0; i < 8; i++) {
		B0 = mem_get_byte(first_byte_addr + i * 2);
		B1 = mem_get_byte(first_byte_addr + i * 2 + 1);
		tile_set_line_VRAM(i, B0, B1, tile_matrix);
	}
	return 0;
}

// debug function to display the full content of VRAM
/*static int draw_frame_VRAM()
{
	static uint32_t cpt_w = 0;
	static uint32_t cpt_lg = 0;
	static uint32_t cpt_dg = 0;
	static uint32_t cpt_d = 0;

	for (int l = 0; l < 4; l++) {
		for (int k = 0; k < 128; k++) {
			tile_set_VRAM(0x8000 + 0x800 * l + 16 * k, tile);

			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					switch (tile[i + j * 8]) {
					case 0:
						SDL_SetRenderDrawColor(
							renderer, 0xFF, 0xFF,
							0xFF, 0xFF);
						cpt_w++;
						break;
					case 1:
						SDL_SetRenderDrawColor(
							renderer, 0xAA, 0xAA,
							0xAA, 0xFF);
						cpt_lg++;
						break;
					case 2:
						SDL_SetRenderDrawColor(
							renderer, 0x55, 0x55,
							0x55, 0xFF);
						cpt_dg++;
						break;
					case 3:
						SDL_SetRenderDrawColor(
							renderer, 0x00, 0x00,
							0x00, 0xFF);
						cpt_d++;
						break;
					default:
						break;
					}
					SDL_RenderDrawPoint(renderer, i + 8 * k,
							    j + l * 8);
				}
			}
		}
	}

	//printf("[pixel_distribution][w,lg,dg,d] %u, %u, %u, %u\n", cpt_w, cpt_lg, cpt_dg, cpt_d);

	return 0;
}*/

uint8_t background[256 * 256] = {};

static int draw_frame_SCREEN()
{
	// DBG: display BG ///////////////////////////////////////
	// TODO: display only screen, not full background
	int x, y, color32;
	for (y = 0; y < 256; y++) {
		for (x = 0; x < 256; x++) {
			switch (background[y * 256 + x]) {
			case 0:
				color32 = COLOR32_WHITE;
				break;
			case 1:
				color32 = COLOR32_LIGHTGRAY;
				break;
			case 2:
				color32 = COLOR32_DARKGRAY;
				break;
			case 3:
				color32 = COLOR32_BLACK;
				break;
			default:
				break;
			}
			SDL_Rect rect = {x*scale, y*scale, scale, scale}; // x, y, width, height
			SDL_FillRect(surface, &rect, color32);
		}
	}
	memset(background, 0, 256 * 256);
	SDL_UpdateWindowSurface(window);

	static int frame_cpt = 0;
	//printf("frame_cpt = %d\n", frame_cpt);
	frame_cpt++;

	return 0;
}

static int gpu_set_line_background(uint8_t line)
{
	uint16_t tile_map_addr;
	uint16_t tile_data_addr;
	uint8_t lcdc = mem_get_byte(LCDC);

	// proceed only if background is enable
	if (!(lcdc & 0x01))
		return 0;

	if (lcdc & 0x08)
		tile_map_addr = 0x9C00;
	else
		tile_map_addr = 0x9800;

	if (lcdc & 0x10)
		tile_data_addr = 0x8000;
	else
		tile_data_addr = 0x8800;

	int idx_max = 0;

	/*if (line == 0)
		printf("[bginfo] tile map = 0x%x, tile_data = 0x%x\n",
		       tile_map_addr, tile_data_addr);*/

	// TODO: use SCX & SCY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*if(line % 8 == 1)
		printf("[%d] ", line);*/

	for (int i = 0; i < 20; i++) {
		uint8_t tile_idx =
			mem_get_byte(tile_map_addr + (line / 8) * 32 + i);

		/*int idx = (line/8)*32*64 + (line%8)*256 + i*8;
		printf("%-3x,", tile_idx);*/

		//int idxB = (line/8)*32*64 + (line%8)*256 + i*8;
		uint16_t tile_line_addr =
			tile_data_addr + tile_idx * 16 + (line % 8) * 2;
		uint8_t B0 = mem_get_byte(tile_line_addr);
		uint8_t B1 = mem_get_byte(tile_line_addr + 1);
		int idxPixel = line * 256 + i * 8;
		//printf("%-3d,", idxPixel);
		//printf("%-3x,", idxB);
		//printf("0x%x,", tile_line_addr);
		/*if(line % 8 == 1) {
			//printf("@%x=", tile_line_addr);
			//printf("0x%x,", B0);
			printf("%-3d,", tile_idx);
		}*/

		// TODO: support BG palette
		tile_set_line_background(B0, B1, &background[idxPixel]);
	}
	/*if(line % 8 == 1)
		printf("\n");*/

	return 0;
}

static int gpu_set_line_sprite(uint8_t line)
{
	uint16_t sprite_data_addr = 0x8000;
	uint8_t lcdc = mem_get_byte(LCDC);

	// TODO: manage 8x16 sprites (when bit2 of LCDC == 1)

	// proceed only if sprite is enable
	if (!(lcdc & 0x01))
		return 0;

	// go through each OAM u32 word
	for (int i = 0; i < 40; i++) {
		uint8_t y = mem_get_byte(OAM_ADDR + i * 4) - 16;

		// sprite must be contained on the current line
		if (line >= y && line <= (y + 7)) {
			uint8_t x = mem_get_byte(OAM_ADDR + i * 4 + 1) - 8;
			uint8_t tile_idx = mem_get_byte(OAM_ADDR + i * 4 + 2);
			uint8_t info = mem_get_byte(OAM_ADDR + i * 4 + 3);
			uint8_t layer = info >> 7;
			uint8_t y_flip = (info & 0x40) >> 6;
			uint8_t x_flip = (info & 0x20) >> 5;
			uint8_t palette = (info & 0x10) >> 4;

			uint8_t B0, B1;

			uint16_t tile_addr = sprite_data_addr +
					     tile_idx * 16 /*+ (line%8) * 2*/;

			if (y_flip)
				tile_addr += (8 - line % 8) * 2;
			else
				tile_addr += (line % 8) * 2;

			B0 = mem_get_byte(tile_addr);
			B1 = mem_get_byte(tile_addr + 1);

			int idxPixel = line * 256 + x;
			tile_set_line_sprite(B0, B1, layer, x_flip, palette,
					     &background[idxPixel]);
		}
	}
}

static int cptt;

int gpu_processing(uint8_t op_duration)
{
	static uint8_t mode = 2; // KO if = 0
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
				/*printf("=> trigger VBLANK interrupt #%d\n",
				       cptt);*/
				cptt++;
				uint8_t val = mem_get_byte(IF);
				mem_set_byte(IF, val | INT_VBLANK);

				// dbg functions
				//draw_frame_VRAM();
				draw_frame_SCREEN();
				//dump_VRAM();

        		// Regulate framerate
				time_regulate_framerate();
			} else {
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

				//printf("\n");

				//printf("[%d] line set to %d\n", __LINE__, gpu_line);
				// TODO: move SDL rendering in gpu.c
				//gpu_render_frame();
			} else {
				gpu_line++;
			}

			mem_set_byte(LY, gpu_line);
		}
		break;

	case OAM_ACCESS:
		if (time_gpu > DURATION_OAM) {
			time_gpu -= DURATION_OAM;
			mode = LCD_DRAWING;
			//gpu_set_line_background(gpu_line);
		}
		break;

	case LCD_DRAWING:
		if (time_gpu > DURATION_LCD) {
			time_gpu -= DURATION_LCD;
			mode = HBLANK;
			gpu_set_line_background(gpu_line);
			gpu_set_line_sprite(gpu_line);
		}

		break;

	default:
		printf("[%s] ERROR: invalid mode, should not happen\n",
		       __func__);
	}
}
