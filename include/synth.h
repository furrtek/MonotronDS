#include <nds.h>
#include <maxmod9.h>
#include <math.h>

mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format);

float osc;
float lfo;
float pitch;
float pitchmod;
float filter;
float ratelfo;
float intlfo;
float peak;
float buf0, buf1;
float track;
int mode;
int octave;
int pan;
s16 previous, smpa, smpb, diff;
