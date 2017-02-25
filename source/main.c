// Korg Monotron emulator for NDS
// Beta v2.0
// CC furrtek 2017 - furrtek.org

#include "main.h"
#include "init.h"
#include "controls.h"
#include "synth.h"

int main(void) {
	uint32_t c;
	u16 * bg_ptr;
	u16 * pixel_ptr;

	init();
	
	bg_ptr = bgGetGfxPtr(bg2s);
	BG_PALETTE_SUB[0] = 0x2108;
	
	while (1) {
		swiWaitForVBlank();
		
		// CPU meter, unit = 1.9096us (ovf @ ~8Hz)
		TIMER3_CR = 0;
		TIMER3_DATA = 0;
		TIMER3_CR = TIMER_ENABLE | TIMER_DIV_64;
		mmStreamUpdate();
		// Average for 10 frames
		if (meter_tick) {
			meter_tick--;
			meter_acc += TIMER3_DATA;
		} else {
			meter_tick = 10;
			job_meter = meter_acc / 10;
			meter_acc = 0;
		}
		
		// Clear 96~160 (64px)
		dmaFillHalfWords(0x0000, bgGetGfxPtr(bg2s) + (256 * 96), 256 * 64 * 2);
		resfresh_flag = true;
		
		// Draw waveform
		for (c = 0; c < 256; c++) {
			pixel_ptr = bg_ptr + c + (256 * 159) - waveform[c];
			*pixel_ptr = (u16)0xFFFF;	// White pixel
		}
		
		controls_update();
		
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}
	
	return 0;
}
