#include <SDL2/SDL.h>

#include "cpu.h"
#include "memory.h"

#define KEY_PRESSED 1
#define KEY_NOT_PRESSED 0

struct input_keys {
	uint8_t up;
	uint8_t down;
	uint8_t left;
	uint8_t right;
	uint8_t A;
	uint8_t B;
	uint8_t start;
	uint8_t select;
};

static struct input_keys keys = {};
static struct input_keys keys_prev = {};

static SDL_Event event;

uint8_t input_get()
{
	uint8_t reg = mem_get_byte(P1);
	uint8_t P15 = (reg >> 5) & 0x01;
	uint8_t P14 = (reg >> 4) & 0x01;
	uint8_t val = 0x00;

	if (P14) {
		val |= keys.down << 3;
		val |= keys.up << 2;
		val |= keys.left << 1;
		val |= keys.right;
		/*if(val)
            printf("[P14] val=0x%x, 0x%x\n", val, ~val);*/
	}
	else if (P15) {
		val |= keys.select << 3;
		val |= keys.start << 2;
		val |= keys.B << 1;
		val |= keys.A;
		/*if(val)
            printf("[P15] val=0x%x, 0x%x\n", val, ~val)*/;
	}

	return ~val;
}

void input_init()
{
	memset(&keys, KEY_NOT_PRESSED, sizeof(keys));
	memset(&keys_prev, KEY_NOT_PRESSED, sizeof(keys_prev));
}

void input_scan()
{
	uint8_t key_status;

	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			printf("Exit game\n");
			exit(0);
		}

		if (event.type == SDL_KEYDOWN) {
			key_status = KEY_PRESSED;
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				printf("Exit game\n");
				exit(0);
				break;
			case SDLK_UP:
				keys.up = key_status;
				break;
			case SDLK_DOWN:
				keys.down = key_status;
				break;
			case SDLK_LEFT:
				keys.left = key_status;
				break;
			case SDLK_RIGHT:
				keys.right = key_status;
				break;
			case SDLK_w:
				keys.A = key_status;
				break;
			case SDLK_x:
				keys.B = key_status;
				break;
			case SDLK_SPACE:
				keys.start = key_status;
				break;
			case SDLK_LALT:
				keys.select = key_status;
				break;
			default:
				continue;
			}
		} else if (event.type == SDL_KEYUP) {
			key_status = KEY_NOT_PRESSED;
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				printf("Exit game\n");
				exit(0);
				break;
			case SDLK_UP:
				keys.up = key_status;
				break;
			case SDLK_DOWN:
				keys.down = key_status;
				break;
			case SDLK_LEFT:
				keys.left = key_status;
				break;
			case SDLK_RIGHT:
				keys.right = key_status;
				break;
			case SDLK_w:
				keys.A = key_status;
				break;
			case SDLK_x:
				keys.B = key_status;
				break;
			case SDLK_SPACE:
				keys.start = key_status;
				break;
			case SDLK_LALT:
				keys.select = key_status;
				break;
			default:
				continue;
			}
		}
	}

	// display input change
	if (memcmp(&keys, &keys_prev, sizeof(keys))) {
		memset(&keys_prev, 0, sizeof(keys));
		if (memcmp(&keys, &keys_prev, sizeof(keys)))
			printf("keys changed => U=%d, D=%d, L=%d, R=%d, A=%d, B=%d, START=%d, SELECT=%d\n",
			       keys.up, keys.down, keys.left, keys.right,
			       keys.A, keys.B, keys.start, keys.select);
	}

	keys_prev = keys;
}