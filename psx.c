#include <project.h>
#include "psx.h"
#include "main.h"

enum {
    PSX_SQUARE = 0x8000, PSX_X = 0x4000, PSX_CIRCLE = 0x2000, PSX_TRIANGLE = 0x1000,
    PSX_R1 = 0x800, PSX_L1 = 0x400, PSX_R2 = 0x200, PSX_L2 = 0x100,
    PSX_LEFT = 128, PSX_DOWN = 64, PSX_RIGHT = 32, PSX_UP = 16, PSX_START = 8,
    PSX_ANALOG_L = 4,  //analog stick pushed
    PSX_ANALOG_R = 2,  //analog stick pushed
    PSX_SELECT = 1,
};

const SettingsT psx_defaults = {
    .analog2_mode = ANALOG2MODE_MAPPED,
    .analog1_center_X = 0x80,
    .analog1_center_Y = 0x7a,
    .analog2_center_X = 0x8d,
    .analog2_center_Y = 0x74,
    .analog1_scale_left = 0xd9c0,
    .analog1_scale_right = 0xdc40,
    .analog1_scale_up = 0xbc00,
    .analog1_scale_down = 0xd000,
    .analog2_scale_left = 0xe400,
    .analog2_scale_right = 0xc800,
    .analog2_scale_up = 0xb000,
    .analog2_scale_down = 0xdd00,
    .buttonmap = {
        0,   //home
        PSX_SELECT,     //select
        PSX_START,      //start
        PSX_L1,         //L2
        PSX_R1,         //R2
        PSX_L2,         //L
        PSX_R2,         //R
        PSX_LEFT | ANALOG1_LEFT,
        PSX_RIGHT | ANALOG1_RIGHT,
        PSX_DOWN | ANALOG1_DOWN,
        PSX_UP | ANALOG1_UP,
        PSX_CIRCLE | ANALOG2_RIGHT,     //A
        PSX_X | ANALOG2_DOWN,          //B
        PSX_TRIANGLE | ANALOG2_UP,   //X
        PSX_SQUARE | ANALOG2_LEFT,     //Y
    },
};

uint8 psx_rw(uint8 data);

// return false if no response detected
int psx_cmd(const uint8 *cmd, uint8 *dout, int size) {
    int i;

    (*(reg32*)SCB_sda__DR_CLR)=SCB_sda__MASK;    // PSX ATT = Wii SDA
    CyDelayUs(20);  //CS-to-CLK delay (dual analog controller needs this)

    for(i=0; i<size; i++) {
        dout[i] = psx_rw(cmd[i]);
    }

    (*(reg32*)SCB_sda__DR_SET)=SCB_sda__MASK;
    //CyDelayUs(20);  //delay between commands
    
    return dout[0]==0xff && dout[1]!=0xff;  //Don't rely on dout[2] ACK, it's not always there (sometimes 00 instead of 5A)
}

// ~ 430uS
int psx_poll() {
    static const uint8 pollController[]={0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 din[9];

    if(!psx_cmd(pollController, din, sizeof(pollController)))
        return 0;

    buttons_raw = (din[3] | (din[4]<<8)) ^ 0xffff;
    if(din[1]>=0x50) {
        analog1_raw_X=~din[7];
        analog1_raw_Y=~din[8];
        analog2_raw_X=~din[5];
        analog2_raw_Y=~din[6];
        no_analog=0;
    } else {
        analog1_raw_X = cfg.analog1_center_X;
        analog1_raw_Y = cfg.analog1_center_Y;
        analog2_raw_X = cfg.analog2_center_X;
        analog2_raw_Y = cfg.analog2_center_Y;
        no_analog=1;
    }
    return 1;
}

int psx_id() {
    static const uint8 enterCfg[]=  {0x01, 0x43, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    //static const uint8 dualshockEnable[] = {0x01, 0x44, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 din[10];

    //take IOs (see resetIO for default pin states)
    SCB_scl_SetDriveMode(SCB_scl_DM_STRONG);
    SCB_sda_SetDriveMode(SCB_sda_DM_STRONG);
    Pin_CMD_SetDriveMode(Pin_CMD_DM_STRONG);

    // Entering "escape" mode on DualShocks is sufficient to enable analog sticks.  IDKWTF "dual shock enable" actually does (just sets the LED?)
    // Escape command isn't recognized by Dual Analog controller, mode button needs to be pushed.
    // If other commands are sent too soon after enterCfg, escape mode is aborted. Standard 16ms poll rate is plenty of time.
    if(psx_cmd(enterCfg, din, sizeof(enterCfg)) && din[2]!=0)   //din[2]!=0 guards against false ID from SNES controller 
        return 1;
    return 0;
}
