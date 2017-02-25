#include "main.h"
#include "synth.h"

// Using floats on NDS sucks
mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
	s16 * target = dest;
	int len = length;
	uint32_t osc_rate;
	uint32_t lfo_v, osc_v;
	float flt = 0;
	float pitchlfo = 1.0f;
	float feedback;
	bool plotting;
	
	flt = filter + track;
	
	if (mode == CUTOFF) {
		flt *= (1 - ((1 - ((float)lfo_acc / 4294967295.0)) * intlfo));
	} else if (mode == PITCH) {
		pitchlfo = (1 - ((1 - ((float)lfo_acc / 4294967295.0)) * intlfo));
	}
	if (flt > 0.99f) flt = 0.99f;
	
	feedback = peak + peak / (1.0f - flt);
	osc_rate = (pitch + ribbon + (octave * 12 * 32) + (pitchlfo * 64));		// Here be an OOB bug :)
	osc_v = vco_lut[osc_rate];

	for ( ; len; len--) {
		if (press && mode) {
			
			// Low-pass filter with feedback
			buf0 = buf0 + flt * ((((float)osc_acc / 4294967295.0) * 45000) - buf0 + feedback * (buf0 - buf1)) / 1.8f;
			buf1 = buf1 + flt * (buf0 - buf1);
			
			*target++ = (s16)buf1 - 32767;	// Mono output
			
			// Debug: Store part of waveform
			if (plotting && (plot_x < 256)) {
				waveform[plot_x] = ((u16)buf1 >> 10) << 8;
				plot_x++;
			}
			
			// Oscillator
			if (osc_acc < osc_v) {
				if (resfresh_flag == true) {
					plot_x = 0;
					resfresh_flag = false;
					plotting = true;
				}
				osc_acc = 0xFFFFFFFF;
			} else {
				osc_acc -= osc_v;
			}
		} else {
			// STFU
			*target++ = 0;
		}
		
		// LFO always runs
		lfo_v = lfo_lut[lfo_rate];
		if (lfo_acc < lfo_v)
			lfo_acc = 0xFFFFFFFF;
		else
			lfo_acc -= lfo_v;
	}
	
	return length;
}
