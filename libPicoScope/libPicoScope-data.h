#ifndef __libPicoScope_data__
#define __libPicoScope_data__

#include <libusb-1.0/libusb.h>

#define USB_DIR_IN         0x80
#define PICO_INSTR         0x01
#define PICO_DATA_BUF      0x02
#define SPARTAN_PRG        0x06
#define PICO_BUFFER_OFFSET 20

struct controlTransferVendorSpecific_t {
    int           length;
    unsigned int  wValue;
    unsigned char data[16];
};

struct bulkDataTransfer_t {
    int            endPoint;
    int            length;
    unsigned char  *dataPnt;
    int            timeout;
};

struct setupPacket_t {
    int           length;
    int           endPoint;
    unsigned char *data;
};


// initialization packets
extern struct controlTransferVendorSpecific_t controlTransferInitDevice[566];
extern struct bulkDataTransfer_t              dataInitPicoScope[18];

// functional packets
extern struct setupPacket_t setupPacket_ButtonLight_OFF;
extern struct setupPacket_t setupPacket_ButtonLight_ON;
extern struct setupPacket_t setupPacket_ButtonLight_READY;
extern struct setupPacket_t setupPacket_READY;
extern struct setupPacket_t setupPacket_SHUTDOWN;
extern struct setupPacket_t setupPacket_SET_TIMEBASE;
extern struct setupPacket_t setupPacket_RUN_BLOCK;
extern struct setupPacket_t setupPacket_collectSamples[3];
extern struct setupPacket_t setupPacket_READ_SAMPLES;

extern short         picoscope_URB_values_to_User[8][256];
extern unsigned char picoscope_voltage_range[8];
extern unsigned char picoscope_voltage_range_tail[8][2];

#endif

