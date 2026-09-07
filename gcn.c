#include <project.h>
#include "gcn.h"
#include "main.h"
#include "config.h"

int joybus_cmd(uint32 cmd, uint32 cmdlen, uint32 *buf);

enum { GCN_LEFT=1, GCN_RIGHT=2, GCN_DOWN=4, GCN_UP=8, GCN_Z=16, GCN_R=32, GCN_L=64,
    GCN_A=1<<8, GCN_B=1<<9, GCN_X=1<<10, GCN_Y=1<<11, GCN_START=1<<12 };

const SettingsT gcn_defaults = {
    .analog2_mode = ANALOG2MODE_MAPPED,
    .analog1_center_X = 0x79,
    .analog1_center_Y = 0x89,
    .analog2_center_X = 0x84,
    .analog2_center_Y = 0x7f,
    .analog1_scale_left = 0x12a40,
    .analog1_scale_right = 0x13000,
    .analog1_scale_up = 0x0f000,
    .analog1_scale_down = 0x110c0,
    .analog2_scale_left = 0x12a00,
    .analog2_scale_right = 0x16400,
    .analog2_scale_up = 0x11800,
    .analog2_scale_down = 0x12700,
    .L_trigger_level = 0x38,
    .R_trigger_level = 0x38,
    .buttonmap = {
        0,    //home
        AND_MAP | GCN_Z | GCN_START,    //select
        GCN_START,                      //start
        0,                              //L2
        GCN_Z,                          //R2
        GCN_L,
        GCN_R,
        GCN_LEFT | ANALOG1_LEFT,
        GCN_RIGHT | ANALOG1_RIGHT,
        GCN_DOWN | ANALOG1_DOWN,
        GCN_UP | ANALOG1_UP,
        GCN_A | ANALOG2_RIGHT,
        GCN_B | ANALOG2_DOWN,
        GCN_X | ANALOG2_UP,
        GCN_Y | ANALOG2_LEFT,
    },
};

static int32 L_trigger_zero;
static int32 R_trigger_zero;

// ~ 360uS
int gcn_poll() {
    uint32 buf[2];

    if(joybus_cmd(0x40c002, 24, buf)!=64) //0100 0000 1100 0000 0000 0010
        return 0;

    buttons_raw = (buf[1]>>16) & 0x1f7f;
    analog1_raw_Y =  buf[1];
    analog1_raw_X = ~buf[1] >> 8;
    analog2_raw_Y =  buf[0] >> 16;
    analog2_raw_X = ~buf[0] >> 24;

    R_trigger = (int32)(buf[0] & 0xff);
    if(R_trigger < R_trigger_zero)
        R_trigger_zero = R_trigger;
    R_trigger -=  R_trigger_zero;
    if(R_trigger > cfg.R_trigger_level)
        buttons_raw |= GCN_R;

    L_trigger = (int32)((buf[0]>>8) & 0xff);
    if(L_trigger < L_trigger_zero)
        L_trigger_zero = L_trigger;
    L_trigger -= L_trigger_zero;
    if(L_trigger > cfg.L_trigger_level)
        buttons_raw |= GCN_L;

    return 1;
}

int gcn_id() {
    uint32 buf[2];

    L_trigger_zero = 0x40;
    R_trigger_zero = 0x40;
    return gcn_poll();
}
