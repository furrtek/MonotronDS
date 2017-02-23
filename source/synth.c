#include "synth.h"
#include "main.h"
#include "lut_lfo.h"

// Everything sucks
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
	s16 *target = dest;
	float flt = 0;
	float pitchlfo = 1.0f;
	uint32_t lfo_v;
	u16 * t;
	bool plotting;
	
	int len = length;
	
	if (mode == CUTOFF) {
		flt = (filter + track) * (1 - ((1 - ((float)lfo_acc / 4294967295.0)) * intlfo));
		pitchlfo = 1.0f;
	} else if (mode == PITCH) {
		pitchlfo = (1 - ((1 - ((float)lfo_acc / 4294967295.0)) * intlfo));
		flt = filter + track;
	}
	if (flt > 0.99f) flt = 0.99f;

	for ( ; len; len--) {
		//intlfo=0: flt=filter*1
		//intlfo=1: flt=filter*lfo
		
		if (press && mode) {

			// Low-pass filter with feedback
			float fb = peak + peak / (1.0f - flt);
			buf0 = buf0 + flt * ((osc * 80) - buf0 + fb * (buf0 - buf1)) / 1.8f;
			buf1 = buf1 + flt * (buf0 - buf1);

			//*target++ = (buf1 * pan) / 256;
			//*target++ = (buf1 * (255 - pan)) / 256;
			*target++ = buf1;
			
			// Debug: Plot waveform
			if (plotting && (i < 200)) {
				t = bgGetGfxPtr(bg2s) + i - (((u16)buf1 / 512) * 256) + (256 * 128) + 22;
				*t = (u16)0xFFFF;
				i++;
			}
			
			// Oscillator
			osc -= (pitch * pitchmod * pitchlfo * pow(2, octave));
			if (osc < 0.1f) {
				if (plot_request == true) {
					i = 0;
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
