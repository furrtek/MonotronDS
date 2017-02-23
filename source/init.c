#include "init.h"
#include "main.h"
#include "synth.h"

// Gfx:
#include "knob.h"
#include "layout.h"
#include "switch.h"

void init() {
	mm_ds_system mmsys;
	mm_stream mmstream;
	
	// Maxmod init
	mmsys.mod_count = 0;
	mmsys.samp_count = 0;
	mmsys.mem_bank = 0;
	mmsys.fifo_channel = FIFO_MAXMOD;
	mmInit(&mmsys);
	
	mmstream.sampling_rate = 25000;				// 25khz
	mmstream.buffer_length = 1200;				// 1200 samples
	mmstream.callback = on_stream_request;		// Set callback function
	mmstream.format = MM_STREAM_16BIT_MONO;		// Mono 16-bit
	mmstream.timer = MM_TIMER0;					// Use hardware timer 0
	mmstream.manual = true;						// Use manual filling
	mmStreamOpen(&mmstream);
	
	// Video init
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);	
	
	vramSetBankA(VRAM_A_MAIN_BG);		// 128kB
	vramSetBankE(VRAM_B_MAIN_SPRITE);	// 128kB
	vramSetBankC(VRAM_C_SUB_BG);		// 128kB
	vramSetBankD(VRAM_D_SUB_SPRITE);	// 128kB
	
	lcdMainOnBottom();
	
	PrintConsole * console = consoleInit(0, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true);
	consoleSetWindow(console, 0, 0, 32, 10);
	consoleSelect(console);
	
	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);
	
	gfx_knob = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_Bmp);
	dmaCopy(knobBitmap, gfx_knob, 32*32*2);
	
	gfx_switch = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_Bmp);
	dmaCopy(switchBitmap, gfx_switch, 16*16*2);
	
	int bg2 = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 1, 0);
	dmaCopy(layoutBitmap, bgGetGfxPtr(bg2), 256*192*2);
	
	bg2s = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 1, 0);
	dmaFillHalfWords(0xFFFF, bgGetGfxPtr(bg2s), 256*192*2);
	bgSetPriority(bg2s, 3);
}
