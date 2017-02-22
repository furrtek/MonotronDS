// Korg Monotron emulator for NDS
// Beta v2.0
// CC furrtek 2017 - furrtek.org

#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

#include "main.h"
#include "synth.h"

#include "knob.h"
#include "layout.h"
#include "switch.h"

u16* gfxk;
u16* gfxs;

int ang=0;

int main(void) {
	touchPosition touch;
	int knobmod=0,knobs,originy=72;
	bool touching=false;
	bool tswitch=false;
	pos knobpos[5] = {{82,71},{114,71},{152,71},{187,71},{224,71}};
	
	press=false;
	draw=false;

	lfo=1.0f;
	pitch=2.64f;
	filter=0.99f;
	ratelfo=0.00006f;
	intlfo=0.0f;
	peak=0.0f;
	track=0;
	mode=1;
	octave=0;
	pan=128;

	mm_ds_system sys;
	sys.mod_count 			= 0;
	sys.samp_count			= 0;
	sys.mem_bank			= 0;
	sys.fifo_channel		= FIFO_MAXMOD;
	mmInit( &sys );

	mm_stream mystream;
	mystream.sampling_rate	= 25000;					// sampling rate = 25khz
	mystream.buffer_length	= 1200;						// buffer length = 1200 samples
	mystream.callback		= on_stream_request;		// set callback function
	mystream.format			= MM_STREAM_16BIT_STEREO;	// format = stereo 16-bit
	mystream.timer			= MM_TIMER0;				// use hardware timer 0
	mystream.manual			= true;						// use manual filling
	mmStreamOpen( &mystream );

	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);	

	vramSetBankA(VRAM_A_MAIN_BG);		// 128K de VRAM pour les BG de main
	vramSetBankE(VRAM_B_MAIN_SPRITE);	// 128K de VRAM pour les sprites de main
	vramSetBankC(VRAM_C_SUB_BG);		// 128K de VRAM pour les BG de sub
	vramSetBankD(VRAM_D_SUB_SPRITE);	// 128K de VRAM pour les sprites de sub

	lcdMainOnBottom();
	
	PrintConsole *console = consoleInit(0,0,BgType_Text4bpp,BgSize_T_256x256,2,0,false,true);
	consoleSetWindow(console,0,0,32,10);
	consoleSelect(console);
	
	oamInit(&oamMain, SpriteMapping_Bmp_1D_128 	, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);
	
	gfxk = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_Bmp);
	dmaCopy(knobBitmap, gfxk, 32*32*2);
	
	gfxs = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_Bmp);
	dmaCopy(switchBitmap, gfxs, 16*16*2);
	
	int bg2 = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 1,0);
	dmaCopy(layoutBitmap, bgGetGfxPtr(bg2), 256*192*2);
	
	bg2s = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 1,0);
	dmaFillHalfWords(0xFFFF, bgGetGfxPtr(bg2s), 256*192*2);
	bgSetPriority(bg2s,3);
	
	oamRotateScale(&oamMain,0,ang+46384,256,256);
	oamRotateScale(&oamMain,1,ang+46384,256,256);
	oamRotateScale(&oamMain,2,ang+46384,256,256);
	oamRotateScale(&oamMain,3,ang+19384,256,256);
	oamRotateScale(&oamMain,4,ang+46384,256,256);

	oamSet(&oamMain,0,82,71,0,15,SpriteSize_32x32,SpriteColorFormat_Bmp,gfxk,0,false,false,false,false,false);
	oamSet(&oamMain,1,114,71,0,15,SpriteSize_32x32,SpriteColorFormat_Bmp,gfxk,1,false,false,false,false,false);
	oamSet(&oamMain,2,152,71,0,15,SpriteSize_32x32,SpriteColorFormat_Bmp,gfxk,2,false,false,false,false,false);
	oamSet(&oamMain,3,187,71,0,15,SpriteSize_32x32,SpriteColorFormat_Bmp,gfxk,3,false,false,false,false,false);
	oamSet(&oamMain,4,224,71,0,15,SpriteSize_32x32,SpriteColorFormat_Bmp,gfxk,4,false,false,false,false,false);
	
	oamSet(&oamMain,5,3,80,0,15,SpriteSize_16x16,SpriteColorFormat_Bmp,gfxs,-1,false,false,false,false,false);

	SetYtrigger( 0 );
	irqEnable( IRQ_VCOUNT );

	while( 1 )
	{
		// wait until line 0
		swiIntrWait( 0, IRQ_VCOUNT);
		
		BG_PALETTE_SUB[0] = 0x4210;
		mmStreamUpdate();
		BG_PALETTE_SUB[0] = 0x2108;

		dmaFillHalfWords(0x0000, bgGetGfxPtr(bg2s), 256*192*2);
		draw=true;

		scanKeys();
		
		if (keysUp() & KEY_TOUCH) {
			touching = false;
			press = false;
		}
		if ((keysUp() & KEY_B) || (keysUp() & KEY_DOWN)) {
			press = false;
		}

		if(keysHeld() & KEY_TOUCH) {
			touchRead(&touch);
			printf(	"\x1b[2;2HPitch:%4.2f LFO:%1.5f\n"
					"  Int:%4.2f   Cutoff:%4.2f  \n"
					"  Peak:%4.2f  Pitchmod:%4.2f\n"
					"  Ang:%5i  Octave:%1.2f  \n"
					"  Mode:%i     Track:%1.3f   \n",
					pitch,ratelfo,intlfo,filter,peak,pitchmod,ang,pow(2,octave),mode,track);
			if (touching == false) {
				knobmod = -1;
				tswitch = false;
				for (knobs=0;knobs<5;knobs++) {
					if ((touch.px > knobpos[knobs].px-20+16) && (touch.px < knobpos[knobs].px+20+16) &&
					(touch.py > knobpos[knobs].py-20+16) && (touch.py < knobpos[knobs].py+20+16)) knobmod = knobs;
				}
				if ((touch.px > 0) && (touch.px < 16) &&
				(touch.py > 72) && (touch.py < 106)) {
					originy = touch.py;
					tswitch = true;
				}
				touching = true;
				lfo=1.0f;	// LFO reset
			}
			if (knobmod > -1) {
				int deltaX = knobpos[knobmod].px+16 - touch.px;
				int deltaY = knobpos[knobmod].py+16 - touch.py;
				ang = (int) ((-16384 * atan2(deltaY,deltaX) / M_PI)-8192) & 0x7FFF;
				if ((ang > 3000) && (ang < 30000)) {
					float val = 1.2f+(-ang/13800.0f);
					oamRotateScale(&oamMain,knobmod,ang+16384,256,256);
					if (knobmod == 0) pitch = (float) (8+(8 * val));
					if (knobmod == 1) ratelfo = (float) (0.001f+(0.001f*val));
					if (knobmod == 2) intlfo = (float) (0.5f+(0.5f*val));
					if (knobmod == 3) filter = (float) (0.5f+(0.5f*val));
					if (knobmod == 4) peak = (float) (0.5f+(0.5f*val));
					//pan = (float) (128+(128*atan2(deltaY, deltaX) / M_PI));
				}
				press = false;
			} else if (tswitch == true) {
				if (((originy - touch.py) < -6) && (mode < 2)) {
					mode++;
					originy=touch.py;
					oamSet(&oamMain,5,3,72+(8*mode),0,15,SpriteSize_16x16,SpriteColorFormat_Bmp,gfxs,-1,false,false,false,false,false);
				} else if (((originy - touch.py) > 6) && (mode > 0)) {
					mode--;
					originy=touch.py;
					oamSet(&oamMain,5,3,72+(8*mode),0,15,SpriteSize_16x16,SpriteColorFormat_Bmp,gfxs,-1,false,false,false,false,false);
				}
			} else {
				if ((touch.px > 12) && (touch.px < 246) && (touch.py > 138) && (touch.py < 170)) {
					pitchmod = 1.0f+(float)(touch.px)/174.0f;
					track=pitchmod/16.0f;
					press = true;
				} else {
					press = false;
				}
			}
		}
		
		if(keysHeld() & KEY_B) {
			press = true;
			pitchmod = 0.5f;
			pitch = 5.0f;
			intlfo = 0.0f;
			filter = 0.99f;
			peak = 0.0f;
		}
		if(keysDown() & KEY_LEFT) {
			if (octave > -2) octave--;
		}
		if(keysDown() & KEY_RIGHT) {
			if (octave < 2) octave++;
		}
		if (keysHeld() & KEY_DOWN) {
			press = true;
		}

		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}
	
	return 0;
}
