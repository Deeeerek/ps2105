/*
  (C) 2009, Tiago Gasiba
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libPicoScope.h"
#include "libPicoScope-data.h"


#define VENDOR_ID     0x0ce9
#define PRODUCT_ID    0x1007
#define USB_DIR_IN    0x80
#define DEBUG         0


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//                                AUXILIARY FUNCTIONS                                   //
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void print_PicoScopeDev_t( struct PicoScopeDev_t dev )
{
#if DEBUG
    printf("devh             = 0x%08x\n",dev.devh);
    printf("device           = 0x%08x\n",dev.device);
    printf("voltageRange     = %d\n",dev.voltageRange);
    printf("couplingType     = %d\n",dev.couplingType);
    printf("triggerDirection = %d\n",dev.triggerDirection);
    printf("triggerThreshold = %d\n",dev.triggerThreshold);
    printf("delayInPercent   = %d\n",dev.delayInPercent);
#endif
}


// Perform a data-bulk transfer to the USB device
int bulkTransfer( libusb_device_handle *devh,     // device handle
                  int                  endPoint,  // endpoint
                  unsigned char        *buf,      // buffer for transfer
                  int                  length,    // buffer length
                  int                  timeout    // timeout (0=infinity)
                )
{
    int  transferred;

    if( libusb_bulk_transfer( devh,
                              endPoint,
                              buf,
                              length,
                              &transferred,
                              timeout // timeout in milliseconds 
                              ) !=0 )
    {
        return -1;
    }
    return transferred;
}


// Perform a data-bulk transfer to the USB device
int sendPacket( struct PicoScopeDev_t *PicoDev,  // PicoScope device
                struct setupPacket_t  *packet,   // packet to be transmitted
                int                   timeout    // timeout (0=infinity)
              )
{
    return bulkTransfer( PicoDev->devh, packet->endPoint, packet->data, packet->length, timeout );
    
}

int programPicoScopePartan( struct PicoScopeDev_t *PicoDev )
{
    int packetNumber = 0;
    while(1){
        if ( (dataInitPicoScope[packetNumber].endPoint==0) &&
             (dataInitPicoScope[packetNumber].length==0) &&
             (dataInitPicoScope[packetNumber].dataPnt==NULL) )
            break;

        if ((dataInitPicoScope[packetNumber].endPoint&USB_DIR_IN)==USB_DIR_IN){
            unsigned char *buffer;

            buffer = (unsigned char*)malloc(dataInitPicoScope[packetNumber].length);
            if( bulkTransfer(PicoDev->devh,
                             dataInitPicoScope[packetNumber].endPoint,
                             buffer,
                             dataInitPicoScope[packetNumber].length,
                             dataInitPicoScope[packetNumber].timeout )
                != dataInitPicoScope[packetNumber].length ){
                return -1;
             }

            free(buffer);
        } else {
            if( bulkTransfer(PicoDev->devh,
                             dataInitPicoScope[packetNumber].endPoint,
                             dataInitPicoScope[packetNumber].dataPnt,
                             dataInitPicoScope[packetNumber].length,
                             dataInitPicoScope[packetNumber].timeout )
                != dataInitPicoScope[packetNumber].length ){
                return -1;
             }
        }
        packetNumber++;
    }
    return 0;
}


int confirmTransfer( struct PicoScopeDev_t *PicoDev, // device handle
                     int                  endPoint,  // endpoint
                     int                  timeout )  // timeout
{
    unsigned char buffer[1] = {0x00};
    if( bulkTransfer(PicoDev->devh,
                     endPoint,
                     buffer,
                     1,
                     timeout )
            != 1 ){
        return -1;
    }

    if (0x01!=*buffer)
        return -1;

    return 0;
}


int blinkButtonLight( struct PicoScopeDev_t *PicoDev, int times, long delay )
{
    for (int ii=0; ii<times; ii++ ){
        usleep(delay);
        if (sendPacket( PicoDev, &setupPacket_ButtonLight_OFF, 0)<0 )
            return -1;
        usleep(delay);
        if (sendPacket( PicoDev, &setupPacket_ButtonLight_ON, 0)<0 )
            return -1;
    }
    usleep(delay);
    return 0;
}



void writeShort( unsigned char *buf, int val )
{
    buf[0] = (val>>8) & 0xff;
    buf[1] = (val>>0) & 0xff;
}

void writeLong( unsigned char *buf, long long val )
{
    buf[0] = (val>>24) & 0xff;
    buf[1] = (val>>16) & 0xff;
    buf[2] = (val>>8 ) & 0xff;
    buf[3] = (val>>0 ) & 0xff;
}

int nextPow2(int x) 
{ 
    if (x == 0) 
        return 0; 
    x--; 
    x = (x >> 1) | x; 
    x = (x >> 2) | x; 
    x = (x >> 4) | x; 
    x = (x >> 8) | x; 
    x = (x >> 16) | x; 
    x++; 
    return x; 
}

void dumpPacket( struct setupPacket_t *pkt )
{
#if DEBUG
    for (int ii=0; ii<pkt->length; ii++ ){
        printf("0x%02x ",pkt->data[ii]);
        if ( (ii%16)==15 ) printf("\n");
    }
    printf("\n");
#endif
}

int setTimeBase( struct PicoScopeDev_t *PicoDev, int timebase )
{

#if DEBUG
    printf("libPicoScope::setTimeBase\n");
    print_PicoScopeDev_t(*PicoDev);
    printf("timebase = %d\n",timebase);
#endif
    int timebase_bitmap = (1<<timebase)-1;
    // configure the sampling rate
    writeLong( setupPacket_SET_TIMEBASE.data+5, timebase_bitmap );
    // configure the voltage range
    setupPacket_SET_TIMEBASE.data[14] = picoscope_voltage_range[PicoDev->voltageRange];
    // configure the coupling type
    if (PicoDev->couplingType==PS2000_DC)
        setupPacket_SET_TIMEBASE.data[14] |= 0x80;  // signal DC coupling
    switch (PicoDev->triggerDirection){
        case PS2000_DISABLED:
            // this basically corresponds to ADC samples very close to zero
            setupPacket_SET_TIMEBASE.data[17] = 0x00;  // trigger disabled
            setupPacket_SET_TIMEBASE.data[20] = picoscope_voltage_range_tail[PicoDev->voltageRange][0];
            setupPacket_SET_TIMEBASE.data[21] = picoscope_voltage_range_tail[PicoDev->voltageRange][1];
            break;
        case PS2000_RISING:
            setupPacket_SET_TIMEBASE.data[17] = 0x20;  // rising edge
            setupPacket_SET_TIMEBASE.data[20] = PicoDev->triggerThreshold;
            setupPacket_SET_TIMEBASE.data[21] = PicoDev->triggerThreshold+6;
            break;
        case PS2000_FALLING:
            setupPacket_SET_TIMEBASE.data[17] = 0x10;  // falling edge
            setupPacket_SET_TIMEBASE.data[20] = PicoDev->triggerThreshold-6;
            setupPacket_SET_TIMEBASE.data[21] = PicoDev->triggerThreshold;
            break;
        default:
            return -1;
            break;
    }
#if (1==DEBUG)
    printf("Set TimeBase\n");
    dumpPacket(&setupPacket_SET_TIMEBASE);
#endif
    sendPacket( PicoDev, &setupPacket_SET_TIMEBASE, 0);
    return 0;
}

int isNullPacket( struct setupPacket_t *pkt )
{
    if ( (pkt->length==0) && (pkt->endPoint==0) && (pkt->data==NULL) )
        return 1;
    return 0;
}

float ps2000_get_voltage_range( PS2000_RANGE voltageRange )
{
    switch (voltageRange) {
        case 0: return 0.1 ; break;
        case 1: return 0.2 ; break;
        case 2: return 0.5 ; break;
        case 3: return 1.0 ; break;
        case 4: return 2.0 ; break;
        case 5: return 5.0 ; break;
        case 6: return 10.0; break;
        case 7: return 20.0; break;
        default:             break;
    }
    return -1.0;
}

double ps2000_abs( double a )
{
    if (a<0)
        return -a;
    return a;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// initialize the PicoScope device
int ps2000_open_unit( struct PicoScopeDev_t *PicoDev )
{
    // initialize libUSB
    if ( libusb_init(NULL)!=0 ){
        PicoDev->devh = NULL;
        return -1;
    }

    // try to open PicoScope
    if ( (PicoDev->devh=libusb_open_device_with_vid_pid (NULL, VENDOR_ID, PRODUCT_ID ))==NULL ){
        libusb_exit(NULL);
        return -1;
    }

    // get device description
    PicoDev->device = libusb_get_device(PicoDev->devh);
    struct libusb_device_descriptor desc;
    if ( libusb_get_device_descriptor(PicoDev->device,&desc)<0 ){
        libusb_close(PicoDev->devh);
        libusb_exit(NULL);
        return -1;
    }

    // determine if the device is already properly initialized or not
    if( desc.bDeviceClass==0xff ){
        // at this point we need to initialize the device properly
        // therefore we send vendor-specific configurations to the
        // device and at the end we confirm that the device is
        // initialized correctly. If not, we return with an error
        int packetNumber = 0;
        while (controlTransferInitDevice[packetNumber].length>0){
            if ( libusb_control_transfer( PicoDev->devh,
                                          LIBUSB_REQUEST_TYPE_VENDOR,                     // requestType
                                          0xa0,                                           // request
                                          controlTransferInitDevice[packetNumber].wValue, // wValue
                                          0,                                              // wIndex
                                          controlTransferInitDevice[packetNumber].data,   // data
                                          controlTransferInitDevice[packetNumber].length, // wLength
                                          0                                               // timeout
                                        )<=0 )
            {
                 libusb_close(PicoDev->devh);
                 libusb_exit(NULL);
                 return -1;
            }
            packetNumber++;
        }

        libusb_unref_device(PicoDev->device);
        libusb_close(PicoDev->devh);

        // try to read again the device description
        for (int newTrial=0; newTrial<10; newTrial++ ){
            sleep(1); // sleep 1 second
            if ( (PicoDev->devh=libusb_open_device_with_vid_pid (NULL, VENDOR_ID, PRODUCT_ID ))==NULL )
                continue;

            PicoDev->device = libusb_get_device(PicoDev->devh);

            if ( libusb_get_device_descriptor(PicoDev->device,&desc)<0 )
                continue;

            if( desc.bDeviceClass!=0xff )
                break;    // device is now properly initialized!

            libusb_unref_device(PicoDev->device);
        }
        if( desc.bDeviceClass==0xff ) {
            // the device still did not wake up with a correct configuration
            // we give up and report a failure...
            libusb_close(PicoDev->devh);
            libusb_exit(NULL);
            return -1;
        }
    }

    // we claim the USB interface in order to talk with the device
    // nobody else will be allowed to talk with the device
    if( libusb_claim_interface(PicoDev->devh,0)<0 ){
        libusb_close(PicoDev->devh);
        libusb_exit(NULL);
        return -1;
    }

    // perform a device reset
    if (libusb_reset_device(PicoDev->devh)!=0 ){
        libusb_close(PicoDev->devh);
        libusb_exit(NULL);
        return -1;
    }

    // program the PicoScope Spartan FPGA
    if (programPicoScopePartan(PicoDev)<0){
        libusb_unref_device(PicoDev->device);
        libusb_close(PicoDev->devh);
        libusb_exit(NULL);
        return -1;
    }

    // at this point, the LED from the device should turn RED

    // confirm that we receive 0x01
    confirmTransfer( PicoDev, 1|USB_DIR_IN, 0);

    // blink PicoScope in order to get a visual feedback that
    // the initialization procedure was concluded with success
    blinkButtonLight(PicoDev,6,70000);

    // initialization is now finished... go to status READY
    sendPacket( PicoDev, &setupPacket_ButtonLight_READY, 0);

    // set default configuration
    PicoDev->voltageRange       = PS2000_5V;
    PicoDev->couplingType       = PS2000_AC;
    PicoDev->triggerDirection   = PS2000_DISABLED;
    PicoDev->triggerThreshold   = 0;
    PicoDev->delayInPercent     = 0;

    return 0;
}



int ps2000_close_unit( struct PicoScopeDev_t *PicoDev )
{
    blinkButtonLight(PicoDev,6,70000);

    // send shutdown packet
    sendPacket( PicoDev, &setupPacket_SHUTDOWN, 0);
    libusb_close(PicoDev->devh);
    libusb_exit(NULL);
    return 0;    
}

int ps2000_run_block( struct PicoScopeDev_t *PicoDev, int timebase, long no_of_values )
{
    int pre_val         = no_of_values;
    int post_val        = 2*(no_of_values+PICO_BUFFER_OFFSET);
    int ii;

#if DEBUG
    print_PicoScopeDev_t(*PicoDev);
#endif
    // FIXME  FIXME FIXME
    if ( setTimeBase( PicoDev, timebase )<0 ){
#if (1==DEBUG)
        printf("Error setting time base\n");
#endif
        return -1;
    }
    // FIXME  FIXME FIXME
    if (PicoDev->delayInPercent==0) {
        writeShort( setupPacket_RUN_BLOCK.data+4, 49110     );
    } else {
        float tmp      = (49149.0 + 0.02*PicoDev->delayInPercent*no_of_values)/2.0;
        int   value    = 2 + 0xfffe&(int)tmp;

        // overwrite pre- and post-values with other values
        // that depend on the amount of delay
        pre_val  = (int)(1+0.01*PicoDev->delayInPercent)*no_of_values;
        post_val = 2*no_of_values;

        writeShort( setupPacket_RUN_BLOCK.data+4, value     );
    }

    writeShort( setupPacket_RUN_BLOCK.data+16, pre_val  );
    writeShort( setupPacket_RUN_BLOCK.data+20, post_val );

#if (1==DEBUG)
    printf("Run Block\n");
    dumpPacket(&setupPacket_RUN_BLOCK);
#endif
    sendPacket(PicoDev,&setupPacket_RUN_BLOCK,0);

    // trigger sampling
    ii=0;
    while (!isNullPacket(&setupPacket_collectSamples[ii])){
        sendPacket(PicoDev,&setupPacket_collectSamples[ii],0);
        ii++;
    }
    usleep(500);  // sleep 500us
    return 0;
}


int ps2000_ready( struct PicoScopeDev_t *PicoDev )
{
    unsigned char buffer[64];

    sendPacket( PicoDev, &setupPacket_READY, 0);
    if( bulkTransfer( PicoDev->devh,
                      PICO_INSTR|USB_DIR_IN,
                      buffer,
                      64,
                      0) <0 )
            return -1;
    unsigned char readyFlag = *(unsigned char*)buffer;
#if (1==DEBUG)
    printf("READY: 0x%02x 0x%02x   %d\n",*(unsigned char*)buffer,*(unsigned char*)(buffer+1),readyFlag);
#endif
    return readyFlag;
}


int ps2000_get_times_and_values( struct PicoScopeDev_t *PicoDev, long *times, short *buffer, long no_of_values )
{
    long numberOfSamples;
    unsigned char *usb_buffer;

    if (NULL==buffer)
        return 0;

    // XXX is this correct?
    if (0==PicoDev->delayInPercent){
        numberOfSamples = nextPow2(no_of_values+1);
    } else {
        numberOfSamples = no_of_values;
    }
    usb_buffer = malloc(numberOfSamples);
    if (NULL==buffer)
        return 0;

    // send packet to prepare reading the samples
    writeLong( setupPacket_READ_SAMPLES.data+1, numberOfSamples );
    sendPacket(PicoDev,&setupPacket_READ_SAMPLES,0);

    fflush(stdout);
    // read the samples
    if (bulkTransfer( PicoDev->devh,             // device handle
                      PICO_DATA_BUF|USB_DIR_IN,  // endpoint
                      usb_buffer,                // buffer for transfer
                      numberOfSamples,           // buffer length
                      0                          // timeout (0=infinity)
                    ) < 0 ){
        free(usb_buffer);
        return -1;
    }

#if (1==DEBUG)
    printf("\nInput Buffer:\n");
#endif
    if (0==PicoDev->delayInPercent){
        #if (1==DEBUG)
        for (long ii=0; ii<numberOfSamples; ii++ ){
            printf("0x%02x ",usb_buffer[ii]);
            if ((ii%16)==15) printf("\n");
        }
        printf("\n");
        #endif
        for (long ii=0; ii<no_of_values; ii++ ){
            buffer[ii] = picoscope_URB_values_to_User[PicoDev->voltageRange][ usb_buffer[ii+PICO_BUFFER_OFFSET] ];
        }
    } else {
        #if DEBUG
            printf("delayInPercent!=0  !!!!!\n");
        #endif
        for (long ii=0; ii<no_of_values; ii++ ){
            buffer[ii] = picoscope_URB_values_to_User[PicoDev->voltageRange][ usb_buffer[ii] ];
        }
    }
    return 0;
}

int ps2000_flash_led( struct PicoScopeDev_t *PicoDev )
{
    return blinkButtonLight( PicoDev, 3, 150000 );
}

int ps2000_get_values( struct PicoScopeDev_t *PicoDev, short *buffer, long no_of_values )
{
    return ps2000_get_times_and_values(PicoDev,NULL,buffer,no_of_values);
}

int ps2000_get_values_mv( struct PicoScopeDev_t *PicoDev, float *buffer, long no_of_values )
{
    static short staticBuffer[25000];  // use a static buffer in order not to loose time with malloc/free
    int   ret;
    float voltageRange = ps2000_get_voltage_range(PicoDev->voltageRange);

    if (voltageRange<0)
        return -1;

    ret = ps2000_get_times_and_values(PicoDev,NULL,staticBuffer,no_of_values);
    for (int ii=0; ii<no_of_values; ii++ )
        buffer[ii] = voltageRange * (staticBuffer[ii]/32767.0);
    return ret;
}


int ps2000_set_channel( struct PicoScopeDev_t *PicoDev, PS2000_COUPLING couplingType, PS2000_RANGE voltageRange )
{
    PicoDev->couplingType = couplingType;
    PicoDev->voltageRange = voltageRange;
    return 0;
}


//                                                             in mV
int ps2000_set_trigger( struct PicoScopeDev_t *PicoDev, float triggerThreshold, PS2000_TDIR triggerDirection, char delayInPercent )
{

    if (triggerDirection==PS2000_DISABLED){
        PicoDev->triggerDirection = triggerDirection;
        PicoDev->delayInPercent   = 0;
        return 0;
    }
    if ( (delayInPercent>0) || (delayInPercent<-100) )
        return -1;
    float  voltageRange = ps2000_get_voltage_range(PicoDev->voltageRange);
    double minDistance  = 1e9;
    int    minIndex;
    double maxVoltage   = (picoscope_URB_values_to_User[PicoDev->voltageRange][0]*voltageRange)/32767.0;
    double minVoltage   = -maxVoltage;

    //printf("minVoltage : %1.2f\n",minVoltage);
    //printf("maxVoltage : %1.2f\n",maxVoltage);

    if ( (triggerThreshold<minVoltage) || (triggerThreshold>maxVoltage) )
        return -1;

    for (int ii=255; ii>=0; ii-- ){  // from most negative to most positive values
        double voltageValue = (picoscope_URB_values_to_User[PicoDev->voltageRange][ii]*voltageRange)/32767.0;
        /*
        printf("%f  %f  d=%f  (%d=>%d) \n",voltageValue,
                                       triggerThreshold,
                                       ps2000_abs( voltageValue-triggerThreshold ),
                                       ii,
                                       picoscope_URB_values_to_User[PicoDev->voltageRange][ii]);
        */
        if ( ps2000_abs( voltageValue-triggerThreshold ) < minDistance ) {
            minDistance = ps2000_abs( voltageValue-triggerThreshold );
            minIndex    = ii;
        }
    }
#if (1==DEBUG)
    printf("Threshold Value = %d (0x%02x) short_value=%d\n",minIndex,minIndex,picoscope_URB_values_to_User[PicoDev->voltageRange][minIndex]);
#endif
    PicoDev->triggerThreshold = minIndex;
    PicoDev->triggerDirection = triggerDirection;
    PicoDev->delayInPercent   = delayInPercent;
    return 0;
}

