#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

enum ctrltype_enum {
	CTRLT_SWITCH = 0,
	CTRLT_KNOB
};

enum ctrl_enum {
	CTRL_NONE = -1,
	CTRL_MODE = 0,
	CTRL_VCO,
	CTRL_LFOR,
	CTRL_LFOI,
	CTRL_VCFC,
	CTRL_VCFP
};

enum mode_enum {
	STANDBY = 0,
	PITCH,
	CUTOFF
};

typedef struct {
	enum ctrltype_enum type;
	int x;
	int y;
	int w;
	int h;
	void * param_ptr;
} control_t;

int bg2s;
u16* gfx_knob;
u16* gfx_switch;
bool press;
bool draw;
enum mode_enum mode;
