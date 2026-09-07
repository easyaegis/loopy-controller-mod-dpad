#include <project.h>
#include "n64.h"
#include "main.h"

int joybus_cmd(uint32 cmd, uint32 cmdlen, uint32 *buf);

enum { N64_A=1<<15, N64_B=1<<14, N64_Z=1<<13, N64_START=1<<12, N64_UP=1<<11, N64_DOWN=1<<10, N64_LEFT=1<<9, N64_RIGHT=1<<8,
    N64_L_R_START=128, N64_L=32, N64_R=16, N64_C_UP=8, N64_C_DOWN=4, N64_C_LEFT=2, N64_C_RIGHT=1 };

const SettingsT n64_defaults = {
    .analog2_mode = ANALOG2MODE_MAPPED,
    .analog1_center_X = 0x7f,
    .analog1_center_Y = 0x80,
    .analog2_center_X = 0x7f,
    .analog2_center_Y = 0x80,
    .analog1_scale_left = 0x13cc0,
    .analog1_scale_right = 0x17e00,
    .analog1_scale_up = 0x12000,
    .analog1_scale_down = 0x13d40,
    .analog2_scale_left = 0,
    .analog2_scale_right = 0,
    .analog2_scale_up = 0,
    .analog2_scale_down = 0,
    .buttonmap = {
        0, //home
        AND_MAP | N64_L | N64_START, //select
        N64_START,                  //start
        0,                          //L2
        0,                          //R2
        N64_L | N64_Z,              //L
        N64_R,                      //R
        N64_LEFT | ANALOG1_LEFT,
        N64_RIGHT | ANALOG1_RIGHT,
        N64_DOWN | ANALOG1_DOWN,
        N64_UP | ANALOG1_UP,
        N64_C_RIGHT,                //A
        N64_C_DOWN,                 //B
        N64_C_UP,                   //X
        N64_C_LEFT,                 //Y
    },
};

// ~ 180uS
int n64_poll() {
    uint32 buf[2];
    if(joybus_cmd(0x80, 8, buf)!=32)
        return 0;

    uint32 btn= buf[0]>>16;
    buttons_raw = (btn | ((btn&N64_L_R_START)<<5)) & 0xff3f;   //N64 controllers are a little goofy.  L+R+Start does a self-calibrate, and a different bit is set for Start.
    analog1_raw_X = (buf[0]>>8)^0x7f;
    analog1_raw_Y = buf[0]^0x80;

    return 1;
}

int n64_id() {
    uint32 buf[2];
    return joybus_cmd(0x00, 8, buf)==24 && (buf[0] & 0xffff00)==0x050000;
}
