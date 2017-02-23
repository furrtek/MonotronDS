#include "main.h"
#include "synth.h"
#include "lut_lfo.h"

// Using floats on NDS sucks
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
	s16 *target = dest;
	float flt = 0;
	float pitchlfo = 1.0f;
	float feedback;
	float osc_rate;
	uint32_t lfo_v;
	u16 * pixel_ptr;
	bool plotting;
	u16 * bg_ptr;
	int len = length;
	
	bg_ptr = bgGetGfxPtr(bg2s);
	
	flt = filter + track;
	
	if (mode == CUTOFF) {
		flt *= (1 - ((1 - ((float)lfo_acc / 4294967295.0)) * intlfo));
	} else if (mode == PITCH) {
		pitchlfo = (1 - ((1 - ((float)lfo_acc / 4294967295.0)) * intlfo));
	}
	if (flt > 0.99f) flt = 0.99f;
	
	feedback = peak + peak / (1.0f - flt);
	osc_rate = (pitch * pitchmod * pitchlfo * pow(2, octave));

	for ( ; len; len--) {
		//intlfo=0: flt=filter*1
		//intlfo=1: flt=filter*lfo
		
		if (press && mode) {

			// Low-pass filter with feedback
			buf0 = buf0 + flt * ((osc * 80) - buf0 + feedback * (buf0 - buf1)) / 1.8f;
			buf1 = buf1 + flt * (buf0 - buf1);
			
			*target++ = buf1;	// Mono output
			
			// Debug: Plot waveform
			if (plotting && (plot_x < 200)) {
				pixel_ptr = bg_ptr + plot_x - (((u16)buf1 / 512) << 8) + (256 * 128) + 22;
				*pixel_ptr = (u16)0xFFFF;	// White pixel
				plot_x++;
			}
			
			// Oscillator
			osc -= osc_rate;
			if (osc < 0.1f) {
				if (plot_request == true) {
					plot_x = 0;
					plot_request = false;
					plotting = true;
				}
				osc = 300.0f;
			}
		} else {
			// STFU
			*target++ = 0;
		}
		
		// LFO
		lfo_v = lut_lfo[lfo_rate];
		if (lfo_acc < lfo_v)
			lfo_acc = 0xFFFFFFFF;
		else
			lfo_acc -= lfo_v;
	}
	
	return length;
}
