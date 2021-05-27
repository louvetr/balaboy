#include "cpu.h"
#include "gpu.h"
#include "memory.h"

#define DURATION_HBLANK 204
#define DURATION_VBLANK 4560
#define DURATION_OAM 80
#define DURATION_LCD 172
#define DURATION_LINE 456

static uint8_t gpu_line = 0;

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
			if (gpu_line >= 143) {
				mode = VBLANK;
				// Trigger VBLANK interrupt
				printf("trigger VBLANK interupt\n");
				uint8_t val = mem_get_byte(IF);
	           	mem_set_byte(IF, val | INT_VBLANK);
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