/*
 (C) 2009, Tiago Gasiba

 The purpose of this program is
     1) to detect if the PicoScope is shown as "Unknown Device" 
     2) if so, then to bring it up as "PicoScope 2000"

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "libPicoScope.h"



int main ( void )
{
    struct PicoScopeDev_t PicoDev;
    short                 samples[30000];


    if (ps2000_open_unit( &PicoDev )<0 ){
        printf("ERROR: could not initialize PicoScope!!!\n");
        exit(-1);
    }

    int numberOfSamples = 512;
    PicoDev.voltageRange = PS2000_20V;

    //ps2000_set_trigger(&PicoDev,10.71,PS2000_FALLING,0);

    ps2000_run_block( &PicoDev, 14, numberOfSamples );

    while (0==ps2000_ready(&PicoDev))
        usleep(50000);  // 50ms

    fflush(stdout);
    ps2000_get_values(&PicoDev,samples,numberOfSamples);

    for (int ii=0; ii<numberOfSamples; ii++ ){
        printf("%-5d\n",samples[ii]);
        //if (15==(ii%16)) printf("\n");
    }
    printf("\n\n");

    ps2000_close_unit(&PicoDev);

    return 0;
}

