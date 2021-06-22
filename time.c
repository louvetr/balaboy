#include <SDL2/SDL.h>

#define SCREEN_FPS 60
#define MS_PER_SEC 1000

const int SCREEN_TICKS_PER_FRAME = MS_PER_SEC / SCREEN_FPS;

static Uint32 start_ticks = 0;

void time_init()
{
    start_ticks = SDL_GetTicks();
}

void time_regulate_framerate()
{

    int frame_ticks = SDL_GetTicks() - start_ticks;
	if (frame_ticks < SCREEN_TICKS_PER_FRAME)
		SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_ticks);

	start_ticks = SDL_GetTicks();

}