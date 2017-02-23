#include <nds.h>
#include <maxmod9.h>
#include <math.h>

mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format);

float osc;
uint32_t lfo_acc;
uint32_t lfo_rate;
float pitch;
float pitchmod;
float filter;
float intlfo;
float peak;
float buf0, buf1;
float track;
int octave;
int i;

