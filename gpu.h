#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef enum {
    HBLANK      = 0,
    VBLANK      = 1,
    OAM_ACCESS  = 2,
    LCD_DRAWING = 3
} gpu_mode;

int gpu_processing(uint8_t op_duration);
int SDL_init();