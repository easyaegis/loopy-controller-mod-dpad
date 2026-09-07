#include <project.h>
#include "snes.h"
#include "main.h"

//A NES controller could work, but its low signal is weak.  Data pin needs to be on an input with no pullup, makes detection tricky.

enum {
    ANALOG_CENTER = 0x80,

    SNES_B=1<<16, SNES_Y=1<<15, SNES_SELECT=1<<14, SNES_START=1<<13,
    SNES_UP=1<<12, SNES_DOWN=1<<11, SNES_LEFT=1<<10, SNES_RIGHT=1<<9,
    SNES_A=1<<8, SNES_X=1<<7, SNES_L=1<<6, SNES_R=1<<5,
    SNES_ALL = 0x1ffe0,
};

const SettingsT snes_defaults = {
    .analog2_mode = ANALOG2MODE_CPP,        //map d-pad to 3DS circle pad
    .analog1_center_X = ANALOG_CENTER,
    .analog1_center_Y = ANALOG_CENTER,
    .analog2_center_X = ANALOG_CENTER,
    .analog2_center_Y = ANALOG_CENTER,
    .analog1_scale_left =   0x10000,
    .analog1_scale_right =  0x10000,
    .analog1_scale_up =     0x10000,
    .analog1_scale_down =   0x10000,
    .analog2_scale_left =   0x10000,
    .analog2_scale_right =  0x10000,
    .analog2_scale_up =     0x10000,
    .analog2_scale_down =   0x10000,
    .buttonmap = {
        0,  //home
        SNES_SELECT,
        SNES_START,
        0,              //L2
        0,              //R2
        SNES_L,
        SNES_R,
        SNES_LEFT,
        SNES_RIGHT,
        SNES_DOWN,
        SNES_UP,
        SNES_A,
        SNES_B,
        SNES_X,
        SNES_Y,
    },
};

uint32 snes_read(int bits);

// ~ 180uS
int snes_poll() {
    uint32 dat = snes_read(17);

    //17th bit should be 0, dpad should be nonzero
    if((dat & 1)==1 || (dat & (SNES_DOWN|SNES_UP|SNES_LEFT|SNES_RIGHT))==0)
        return 0;

    buttons_raw = (dat ^ SNES_ALL) & SNES_ALL;
    analog1_raw_X = ANALOG_CENTER;
    analog1_raw_Y = ANALOG_CENTER;
    analog2_raw_X = ANALOG_CENTER;
    analog2_raw_Y = ANALOG_CENTER;
    return 1;
}

int snes_id() {

    //take IOs (see resetIO for default pin states)
    SCB_scl_SetDriveMode(SCB_scl_DM_STRONG);    //SCL = SNES clock
    SCB_sda_SetDriveMode(SCB_sda_DM_STRONG);    //SDA = SNES latch

    if(snes_poll()) {
        no_analog=1;
        return 1;
    }
    return 0;
}
