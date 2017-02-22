#include "synth.h"
#include "main.h"

mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format) {
	s16 *target = dest;
	float flt=0;
	float pitchlfo=1.0f;

	int len = length;
	int i=0;
	
				if (mode == 2) {
					flt=(filter+track)*(1-((1-lfo)*intlfo));
					pitchlfo=1.0f;
				} else if (mode == 1) {
					pitchlfo=(1-((1-lfo)*intlfo));
					flt=filter+track;
				}
				if (flt > 1.0f) flt = 0.99f;

		for( ; len; len-- )
		{
			//intlfo=0: flt=filter*1
			//intlfo=1: flt=filter*lfo
			
			if ((press == true) && (mode)) {

				float fb = peak + peak/(1.0f - flt);
				buf0 = buf0 + flt * ((osc*80) - buf0 + fb * (buf0 - buf1))/1.8f; //
				buf1 = buf1 + flt * (buf0 - buf1);

				*target++ = (buf1*pan)/256;
				*target++ = (buf1*(255-pan))/256;
				if ((draw == true) && (len > length/2)) {
					u16 *t = bgGetGfxPtr(bg2s)+i-(((u16)buf1/512)*256)+(256*128)+22;
					*t = (u16)0xFFFF;
					i++;
				}
				
				osc-=(pitch*pitchmod*pitchlfo*pow(2,octave));
				if (osc < 0.1f ) osc = 300.0f;

				lfo-=ratelfo;
				if (lfo < 0.01f) lfo=1.0f;
			} else {
					*target++ = 0;
					*target++ = 0;
			}
		}
		
		draw = false;
	
	return length;
}
