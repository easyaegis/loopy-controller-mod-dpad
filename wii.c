#include <project.h>
#include "wii.h"
#include "i2c.h"
#include "main.h"

static const uint8 wrF055[]={0xf0, 0x55};
static const uint8 wrFB00[]={0xfb, 0x00};
static const uint8 wrFE[]={0xfe};
static const uint8 wr00[]={0x00};

enum { WII_R=2, WII_START=4, WII_HOME=8, WII_SEL=16, WII_L=32, WII_DOWN=64, WII_RIGHT=128,
    WII_UP=0x100, WII_LEFT=0x200, WII_ZR=0x400, WII_X=0x800, WII_A=0x1000, WII_Y=0x2000, WII_B=0x4000, WII_ZL=0x8000 };

const SettingsT wii_defaults = {
    .analog2_mode = ANALOG2MODE_MAPPED,
    .analog1_center_X = 0x7f,
    .analog1_center_Y = 0x80,
    .analog2_center_X = 0x7f,
    .analog2_center_Y = 0x80,
    .analog1_scale_left = 0x10800,
    .analog1_scale_right = 0x13100,
    .analog1_scale_up = 0xf040,
    .analog1_scale_down = 0x10500,
    .analog2_scale_left = 0x10700,
    .analog2_scale_right = 0x13e00,
    .analog2_scale_up = 0x10000,
    .analog2_scale_down = 0x0f080,
    .buttonmap = {
        0,
        WII_SEL,
        WII_START,
        WII_ZL,                     //L2
        WII_ZR,                     //R2
        WII_L,
        WII_R,
        WII_LEFT | ANALOG1_LEFT,
        WII_RIGHT | ANALOG1_RIGHT,
        WII_DOWN | ANALOG1_DOWN,
        WII_UP | ANALOG1_UP,
        WII_A | ANALOG2_RIGHT,
        WII_B | ANALOG2_DOWN,
        WII_X | ANALOG2_UP,
        WII_Y | ANALOG2_LEFT,
    },
};

// ~ 540uS
int wii_poll() {
    uint8 buf[6];

    int ok = i2c_read(&buf[0], sizeof(buf));
    ok &= i2c_write(wr00, sizeof(wr00)); //don't short-circuit wr00
    if(!ok || buf[4]==0)   //all returned data is 00 if read is invalid
        return 0;

    buttons_raw = 0xffff ^ ((buf[5]<<8) | buf[4]);
    analog1_raw_X = ~(buf[0]<<2);
    analog1_raw_Y = buf[1]<<2;
    analog2_raw_X = ~((buf[0]&0xc0) | ((buf[1]&0xc0)>>2) | ((buf[2]&0x80)>>4));
    analog2_raw_Y = buf[2]<<3;

    return 1;
}

int wii_id() {
    uint32 id;

    //take IOs (see resetIO for default pin states)
    *(reg32*)CYREG_HSIOM_PORT_SEL3=0xEE;    //change pin function to I2C.  Use weak pullups on IOs (controller should have 1.8Ks too)
    i2c_enable();
    
    if(!i2c_write(wrF055, sizeof(wrF055)))
        return 0;
    if(!i2c_write(wrFB00, sizeof(wrFB00)))
        return 0;
    if(!i2c_write(wrFE, sizeof(wrFE)))
        return 0;
    if(!i2c_read((uint8*)&id, 2))
        return 0;
    if(!i2c_write(wr00, sizeof(wr00)))
        return 0;
    return (id&0xffff)==0x0101;
}
