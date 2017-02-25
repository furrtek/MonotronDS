#include <nds.h>
#include <maxmod9.h>
#include <math.h>

mm_word on_stream_request(mm_word length, mm_addr dest, mm_stream_formats format);

volatile s16 waveform[256];

uint32_t osc_acc;
uint32_t lfo_acc;
uint32_t lfo_rate;
uint32_t pitch;
uint32_t ribbon;
uint32_t octave;
uint32_t plot_x;
float filter;
float intlfo;
float peak;
float buf0, buf1;
float track;

