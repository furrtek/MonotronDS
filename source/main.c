// Korg Monotron emulator for NDS
// Beta v2.0
// CC furrtek 2017 - furrtek.org

#include "main.h"
#include "init.h"
#include "synth.h"

const control_t controls[CONTROLS_COUNT] = {
	{CTRLT_SWITCH, 3, 80, 16, 32, &mode},
	{CTRLT_KNOB, 82, 71, 32, 32, &pitch},
	{CTRLT_KNOB_LED, 114, 71, 32, 32, &lfo_rate},
	{CTRLT_KNOB, 152, 71, 32, 32, &intlfo},
	{CTRLT_KNOB, 187, 71, 32, 32, &filter},
	{CTRLT_KNOB, 224, 71, 32, 32, &peak}
};

int main(void) {
	touchPosition touch;
	enum ctrl_enum control_hit = CTRL_NONE;
	uint32_t c;
	uint32_t touch_x, touch_y;
	int32_t delta_x, delta_y;
	uint32_t keys_down;
	uint32_t keys_held;
	int led;
	void * param_ptr;
	
	int ang = 0;
	int originy = 72;
	bool touching = false;
	
	
	press = false;
	plot_request = false;
	
	lfo_acc = 0xFFFFFFFF;
	pitch=2.64f;
	filter=0.99f;
	lfo_rate = 0;
	intlfo=0.0f;
	peak=0.0f;
	track=0;
	mode = PITCH;
	octave=0;
	i = 0;
	//pan=128;
	
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

		dmaFillHalfWords(0x0000, bgGetGfxPtr(bg2s), 256*192*2);
		plot_request = true;

		scanKeys();
		keys_down = keysDown();
		keys_held = keysHeld();
		
		if (keysUp() & KEY_TOUCH) {
			touching = false;
			press = false;
		}
		if ((keysUp() & KEY_B) || (keysUp() & KEY_DOWN)) {
			press = false;
		}

		if (keys_held & KEY_TOUCH) {
			touchRead(&touch);
			
			// Debug
			printf(	"\x1b[2;2HPitch:%4.2f LFO:%4X\n"
					"  Int:%4.2f   Cutoff:%4.2f  \n"
					"  Peak:%4.2f  Pitchmod:%4.2f\n"
					"  Ang:%5i  Octave:%1.2f  \n"
					"  Mode:%u     Track:%1.3f   \n",
					pitch, (unsigned int)(lfo_rate >> 16), intlfo, filter, peak, pitchmod, ang, pow(2,octave), (uint16_t)mode, track);
			
			touch_x = touch.px;
			touch_y = touch.py;
			
			// Touch start, scan hitboxes
			if (!touching) {
				control_hit = CTRL_NONE;
				
				for (c = 0; c < CONTROLS_COUNT; c++) {
					if ((touch_x > controls[c].x) && (touch_x < controls[c].x + controls[c].w) &&
					(touch_y > controls[c].y) && (touch_y < controls[c].y + controls[c].h))
						control_hit = c;
				}
				originy = touch_y;		// For switch
				touching = true;
				lfo_acc = 0xFFFFFFFF;	// Reset LFO
			}
			
			if (control_hit > CTRL_MODE) {
				delta_x = controls[control_hit].x + 16 - touch_x;
				delta_y = controls[control_hit].y + 16 - touch_y;
				
				ang = (int)((-16384 * atan2(delta_y, delta_x) / M_PI) - 8192) & 0x7FFF;
				
				if ((ang > 3000) && (ang < 30000)) {
					float val = 1.2f + (-ang / 13800.0f);
					oamRotateScale(&oamMain, control_hit, ang + 16384, 256, 256);
					
					param_ptr = controls[control_hit].param_ptr;
					
					if (control_hit == CTRL_VCO) *(float*)param_ptr = (float)(8 + (8 * val));			// Pitch
					if (control_hit == CTRL_LFOR) *(uint32_t*)param_ptr = (uint32_t)((1 + val) * 512);	// LFO rate LUT index
					if (control_hit == CTRL_LFOI) *(float*)param_ptr = (float)(0.5f + (0.5f * val));	// LFO intensity
					if (control_hit == CTRL_VCFC) *(float*)param_ptr = (float)(0.5f + (0.5f * val));	// Filter cutoff
					if (control_hit == CTRL_VCFP) *(float*)param_ptr = (float)(0.5f + (0.5f * val));	// Filter peak
					
					//pan = (float) (128+(128*atan2(deltaY, deltaX) / M_PI));
				}
				press = false;
			} else if (control_hit == CTRL_MODE) {
				delta_y = originy - touch_y;
				if ((delta_y < -6) && (mode < CUTOFF)) {
					mode++;
					originy = touch_y;
					oamSet(&oamMain, 0, 3, 72 + (8 * mode), 0, 15, SpriteSize_16x16, SpriteColorFormat_Bmp,
							gfx_switch, -1, false, false, false, false, false);
				} else if ((delta_y > 6) && (mode > STANDBY)) {
					mode--;
					originy = touch_y;
					oamSet(&oamMain, 0, 3, 72 + (8 * mode), 0, 15, SpriteSize_16x16, SpriteColorFormat_Bmp,
							gfx_switch, -1, false, false, false, false, false);
				}
			} else {
				if ((touch_x > 12) && (touch_x < 246) && (touch_y > 138) && (touch_y < 170)) {
					pitchmod = 1.0f + (float)(touch_x) / 174.0f;
					track = pitchmod / 16.0f;
					press = true;
				} else {
					press = false;
				}
			}
		}
		
		// Update LED knob
		if (mode == STANDBY) {
			led = 0;
		} else {
			if (lfo_rate < 512) {
				led = lfo_acc >> 29;
			} else {
				led = 3 + ((lfo_rate - 512) >> 7);
			}
		}
		oamSetGfx(&oamMain, CTRL_LFOR, SpriteSize_32x32, SpriteColorFormat_Bmp, gfx_knob_led[led]);
		
		// Debug
		if (keys_held & KEY_B) {
			press = true;
			pitchmod = 0.5f;
			pitch = 5.0f;
			intlfo = 0.0f;
			filter = 0.99f;
			peak = 0.0f;
		}
		if (keys_down & KEY_LEFT) {
			if (octave > -2) octave--;
		}
		if (keys_down & KEY_RIGHT) {
			if (octave < 2) octave++;
		}
		if (keys_held & KEY_DOWN) {
			press = true;
		}

		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}
	
	return 0;
}
