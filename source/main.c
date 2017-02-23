// Korg Monotron emulator for NDS
// Beta v2.0
// CC furrtek 2017 - furrtek.org

#include "main.h"
#include "init.h"
#include "controls.h"
#include "synth.h"

int main(void) {
	press = false;
	plot_request = false;
	
	lfo_acc = 0xFFFFFFFF;
	pitch = 2.64f;
	filter = 0.99f;
	lfo_rate = 0;
	intlfo = 0.0f;
	peak = 0.0f;
	track = 0;
	mode = STANDBY;
	octave = 0;
	plot_x = 0;
	touching = false;
	control_hit = CTRL_NONE;
	
	init();
	
	//timerStart(0, TIMER_DIV_1, u16 ticks, null);
	
	SetYtrigger(0);
	irqEnable(IRQ_VCOUNT);
	
	while (1) {
		// Wait for line 0
		swiIntrWait(0, IRQ_VCOUNT);
		
		// Job meter
		BG_PALETTE_SUB[0] = 0x4210;
		mmStreamUpdate();
		BG_PALETTE_SUB[0] = 0x2108;
		
		dmaFillHalfWords(0x0000, bgGetGfxPtr(bg2s), 256 * 192 * 2);
		plot_request = true;
		
		controls_update();
		
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}
	
	return 0;
}
