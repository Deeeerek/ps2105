#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* Definition of PS2000 driver routines on Linux */
#include <libps2000/ps2000.h>
#include "ps2000_struct.h"
#include "ps2000.h"


#define Sleep(x) usleep(1000*(x))

#define SPACES                       \
    for ( int ii=0; ii<5; ii++ ){    \
        printf("\n");                \
        fflush(stdout);              \
    }

int main ( int argc, char **argv )
{

    int delayValue = 0;
    int bufferSize = 512;
    if (argc==3){
        bufferSize = atoi( argv[1] );
        delayValue = atoi( argv[2] );
    } else {
        bufferSize = 512;
        delayValue = 0;
    }
    printf("bufferSize = %d\n",bufferSize);
    printf("delayValue = %d\n",delayValue);

    UNIT_MODEL unitOpened;
    char       line[80];
    char       description [6][25]=  { "Driver Version",
                                       "USB Version",
                                       "Hardware Version",
                                       "Variant Info",
                                       "Serial",
                                       "Error Code"
                                     };

    printf ( "ps2000_open_unit\n" );
    unitOpened.handle = ps2000_open_unit ();

    SPACES;
    printf ( "ps2000_set_channel\n" );
    //                         unit         channel=0   enabled(0/1)  dc(1)/ac(0)   range(3..10)
    ps2000_set_channel( unitOpened.handle, 0,               1,            0,           10   );

    SPACES;
    printf ( "ps2000_set_trigger\n" );
    short auto_trigger_ms = 0;
    //ps2000_set_trigger ( unitOpened.handle, PS2000_CHANNEL_A, 0x4567, PS2000_FALLING , delayValue, auto_trigger_ms );
    //ps2000_set_trigger ( unitOpened.handle, PS2000_CHANNEL_A, 0x0123, PS2000_RISING, delayValue, auto_trigger_ms );

    SPACES;
    printf ( "ps2000_get_timebase\n" );
    // find the maximum number of samples, the time interval (in time_units),
    // the most suitable time units, and the maximum oversample at the current timebase
    long 	time_interval;
    short 	time_units;
    short   oversample = 1;
    long 	max_samples;
    short   timebase = 14;
    long    no_of_samples = bufferSize;
    /*
    while (!ps2000_get_timebase ( unitOpened.handle,
                                  timebase,
                                  no_of_samples,
                                  &time_interval,
                                  &time_units,
                                  oversample,
                                  &max_samples)
         )
        timebase++;
    */
    printf("timebase      = %d (%x)\n",timebase,timebase);
    printf("no_of_samples = %d (%x)\n",no_of_samples, no_of_samples);
    //printf("time_interval = %d (%x)\n",time_interval,time_interval);
    //printf("time_units    = %d (%x)\n",time_units,time_units);
    //printf("oversample    = %d (%x)\n",oversample,oversample);
    //printf("max_samples   = %d (%x)\n",max_samples,max_samples);

    SPACES;
    printf ( "ps2000_run_block\n" );
    long time_indisposed_ms;
    ps2000_run_block ( unitOpened.handle, no_of_samples, timebase, oversample, &time_indisposed_ms );

    int flag = 0;
    do {
        SPACES;
        printf ( "ps2000_ready\n" );
        flag = ps2000_ready ( unitOpened.handle );
        if (flag) break;
        Sleep ( 100 );
    } while (!flag);

    SPACES;
    printf ( "ps2000_stop\n" );
    ps2000_stop ( unitOpened.handle );


    SPACES;
    printf ( "ps2000_get_values\n" );
    short  buffer[bufferSize];
    long   times[bufferSize];
    short  overflow;
    ps2000_get_values ( unitOpened.handle,
                        //times,
                        buffer,
                        NULL,
                        NULL,
                        NULL,
                        &overflow,
                        //time_units,
                        no_of_samples );

  for ( int ii=0; ii<bufferSize; ii++ ){
  //for ( int ii=0; ii<10; ii++ ){
      printf("s: %-5d \n", buffer[ii]);
      //if ((ii%10)==9) printf("\n");
  }
  printf("\n");



    SPACES;
    printf ( "ps2000_close_unit\n");
    ps2000_close_unit ( unitOpened.handle );

    return 0;
}
