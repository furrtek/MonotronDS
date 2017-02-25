#include "main.h"
#include "controls.h"
#include "synth.h"

const control_t controls[CONTROLS_COUNT] = {
	{CTRLT_SWITCH, 3, 72, 16, 32, &mode},
	{CTRLT_KNOB, 82, 71, 32, 32, &pitch},
	{CTRLT_KNOB_LED, 114, 71, 32, 32, &lfo_rate},
	{CTRLT_KNOB, 152, 71, 32, 32, &intlfo},
	{CTRLT_KNOB, 187, 71, 32, 32, &filter},
	{CTRLT_KNOB, 224, 71, 32, 32, &peak}
};

void controls_update() {
	touchPosition touch;
	uint32_t touch_x, touch_y;
	int32_t delta_x, delta_y;
	uint32_t keys_down, keys_held;
	uint32_t c;
	int ang = 0;
	int led;
	void * param_ptr;

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
		printf(	"\x1b[2;2HPitch:%3X  LFO:%3X\n"
				"  Int:%4.2f   Cutoff:%4.2f  \n"
				"  Peak:%4.2f  Ribbon:%u  \n"
				"  Ang:%5i  Octave:%u  \n"
				"  Mode:%u     Track:%1.3f   \n"
				"  Meter:%u%%    ",
				(unsigned int)pitch, (unsigned int)lfo_rate,
				intlfo, filter, peak, (unsigned int)ribbon, ang, (unsigned int)octave,
				(uint16_t)mode, track, (job_meter * 100) / 0x22AC);
		
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
			// Knobs
			delta_x = controls[control_hit].x + 16 - touch_x;
			delta_y = controls[control_hit].y + 16 - touch_y;
			
			ang = (int)((-16384 * atan2(delta_y, delta_x) / M_PI) - 8192) & 0x7FFF;
			
			if ((ang > 3000) && (ang < 30000)) {
				float val = 1.2f + (-ang / 13800.0f);
				oamRotateScale(&oamMain, control_hit, ang + 16384, 256, 256);
				
				param_ptr = controls[control_hit].param_ptr;
				
				if (control_hit == CTRL_VCO) *(uint32_t*)param_ptr = (uint32_t)((1 + val) * 651);	// Pitch
				if (control_hit == CTRL_LFOR) *(uint32_t*)param_ptr = (uint32_t)((1 + val) * 512);	// LFO rate LUT index
				if (control_hit == CTRL_LFOI) *(float*)param_ptr = (float)(0.5f + (0.5f * val));	// LFO intensity
				if (control_hit == CTRL_VCFC) *(float*)param_ptr = (float)(0.5f + (0.5f * val));	// Filter cutoff
				if (control_hit == CTRL_VCFP) *(float*)param_ptr = (float)(0.5f + (0.5f * val));	// Filter peak
			}
			press = false;
		} else if (control_hit == CTRL_MODE) {
			// Switch
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
			// Ribbon (234px)
			if ((touch_x >= 12) && (touch_x < 246) && (touch_y > 138) && (touch_y < 170)) {
				ribbon = touch_x - 12;
				//track = ribbon / 16.0f;
				press = true;
			} else {
				press = false;
			}
		}
	}
	
	// Update LED knob
	if (mode == STANDBY) {
		// Off
		led = 0;
	} else {
		if (lfo_rate < 512) {
			// Animate
			led = lfo_acc >> 29;
		} else {
			// Anti-aliasing: fade in if rate above threshold
			led = 3 + ((lfo_rate - 512) >> 7);
		}
	}
	oamSetGfx(&oamMain, CTRL_LFOR, SpriteSize_32x32, SpriteColorFormat_Bmp, gfx_knob_led[led]);
	
	// Test tone
	if (keys_held & KEY_B) {
		press = true;
		ribbon = 127;
		pitch = 5.0f;
		intlfo = 0.0f;
		filter = 0.99f;
		peak = 0.0f;
	}
	
	// Octave shift
	if (keys_down & KEY_LEFT) {
		if (octave > -2) octave--;
	}
	if (keys_down & KEY_RIGHT) {
		if (octave < 2) octave++;
	}
	
	// Sustain
	if (keys_held & KEY_DOWN) {
		press = true;
	}
}
