/*
  (C) 2009, Tiago Gasiba
*/

#ifndef _libPicoScope_
#define _libPicoScope_

#include <libusb-1.0/libusb.h>

#define PS2000_MAX_VALUE 32767
#define PS2000_MIN_VALUE -32767
#define PS2000_LOST_DATA -32768

typedef enum enPS2000Range
{
	PS2000_100MV  = 0,
    PS2000_200MV  = 1,
    PS2000_500MV  = 2,
	PS2000_1V     = 3,
    PS2000_2V     = 4,
    PS2000_5V     = 5,
	PS2000_10V    = 6,
    PS2000_20V    = 7,
}	PS2000_RANGE;

typedef enum enPS2000Coupling
{
	PS2000_DC  = 0,
    PS2000_AC  = 1,
}	PS2000_COUPLING;

typedef enum enPS2000TriggerDirection
{
  PS2000_RISING   = 0,
  PS2000_FALLING  = 1,
  PS2000_DISABLED = 2,
}	PS2000_TDIR;



struct PicoScopeDev_t {
    struct libusb_device_handle    *devh;                // USB device handle
    struct libusb_device           *device;              // USB device descriptor
    PS2000_RANGE                   voltageRange;         // selected voltage range
    PS2000_COUPLING                couplingType;         // AC/DC coupling
    PS2000_TDIR                    triggerDirection;     // rising or falling
    unsigned char                  triggerThreshold;     // threshold value for trigger (ADC value)
    char                           delayInPercent;       // ]-100, 0[ of the capture window
};




int ps2000_open_unit( struct PicoScopeDev_t *PicoDev );
int ps2000_close_unit( struct PicoScopeDev_t *PicoDev );
int ps2000_run_block( struct PicoScopeDev_t *PicoDev, int timebase, long no_of_values );
int ps2000_ready( struct PicoScopeDev_t *PicoDev );
int ps2000_get_times_and_values( struct PicoScopeDev_t *PicoDev, long *times, short *buffer, long no_of_values );
int ps2000_flash_led( struct PicoScopeDev_t *PicoDev );
int ps2000_get_values( struct PicoScopeDev_t *PicoDev, short *buffer, long no_of_values );
int ps2000_get_values_mv( struct PicoScopeDev_t *PicoDev, float *buffer, long no_of_values );
int ps2000_set_channel( struct PicoScopeDev_t *PicoDev, PS2000_COUPLING coupingType, PS2000_RANGE voltageRange );
int ps2000_set_trigger( struct PicoScopeDev_t *PicoDev, float triggerThreshold, PS2000_TDIR triggerDirection, char delayInPercentage );

#endif

