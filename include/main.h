#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

#define CONTROLS_COUNT 6
#define KNOB_SIZE 32

enum ctrltype_enum {
	CTRLT_SWITCH = 0,
	CTRLT_KNOB,
	CTRLT_KNOB_LED
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

extern const control_t controls[CONTROLS_COUNT];
int bg2s;
u16 * gfx_switch;
u16 * gfx_knob;
u16 * gfx_knob_led[8];
bool press;
enum ctrl_enum control_hit;
bool plot_request;
enum mode_enum mode;
bool touching;
int originy;
