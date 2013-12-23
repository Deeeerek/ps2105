/*
    (C) 2009, Tiago Gasiba

    will require some cleanup of the headers afterwards...
*/
#define _GNU_SOURCE
#define CONFIG_X86_BSWAP

#include <stdio.h>
#include <linux/usbdevice_fs.h>
#include <errno.h>
#include <string.h>

#define	_SYS_IOCTL_H	1
#include <bits/ioctls.h>
#include <linux/usb/ch9.h>

#define NO_POINTERS 1


void dump_urb( struct usbdevfs_urb urb, int dumpData )
{
    unsigned int buffer_length = urb.buffer_length;
    printf("    type                 = ");
    fflush(stdout);

    switch (urb.type) {
        case USBDEVFS_URB_TYPE_ISO:
            printf("USBDEVFS_URB_TYPE_ISO\n");
            fflush(stdout);
            break;

        case USBDEVFS_URB_TYPE_INTERRUPT:
            printf("USBDEVFS_URB_TYPE_INTERRUPT\n");
            fflush(stdout);
            break;

        case USBDEVFS_URB_TYPE_BULK:
        case USBDEVFS_URB_TYPE_CONTROL:
            if (USBDEVFS_URB_TYPE_BULK==urb.type)
                printf("USBDEVFS_URB_TYPE_BULK\n");
            else
            if (USBDEVFS_URB_TYPE_CONTROL==urb.type)
                printf("USBDEVFS_URB_TYPE_CONTROL\n");

            // start dumping transfer
            fflush(stdout);
            if (USB_DIR_IN==(urb.endpoint&USB_DIR_IN))
                printf("    endpoint             = %d|USB_DIR_IN\n",urb.endpoint^USB_DIR_IN);
            else
                printf("    endpoint             = %d|USB_DIR_OUT\n",urb.endpoint);
            fflush(stdout);
            if( 0!=urb.status ){
                switch (urb.status){
                    case -ENOENT:
                        printf("    status               = -ENOENT  : URB was synchronously unlinked by usb_unlink_urb\n");
                        break;
                    case -EINPROGRESS:
                        printf("    status               = -EINPROGRESS: URB still pending, no results yet\n");
                        break;
                    case -EPROTO:
                        printf("    status               = -EPROTO: bitstuff error / no response within turn-around time / unknown error\n");
                        break;
                    case -EILSEQ:
                        printf("    status               = -EILSEQ: CRC mismatch / no response within turn-around time / unknown error\n");
                        break;
                    case -ETIME:
                        printf("    status               = -ETIME: no response within turn-around time\n");
                        break;
                    case -ETIMEDOUT:
                        printf("    status               = -ETIMEDOUT: timeout expired before the transfer completed\n");
                        break;
                    case -EPIPE:
                        printf("    status               = -EPIPE: Endpoint stalled\n");
                        break;
                    case -ECOMM:
                        printf("    status               = -ECOMM: During IN transf, host controller received data from endpoint faster than could writte to memory\n");
                        break;
                    case -ENOSR:
                        printf("    status               = -ENOSR: During OUT transf, host controller couldnt get data fast enough to keep up with the USB data rate\n");
                        break;
                    case -EOVERFLOW:
                        printf("    status               = -EOVERFLOW: amount returned by EP greater than either the max pckt size of EP or the remaining buff size.\n");
                        break;
                    case -EREMOTEIO:
                        printf("    status               = -EREMOTEIO: data read from EP didnt fill the buffer, and URB_SHORT_NOT_OK was set in urb->transfer_flags.\n");
                        break;
                    case -ENODEV:
                        printf("    status               = -ENODEV: Device was removed\n");
                        break;
                    case -EXDEV:
                        printf("    status               = -EXDEV: ISO transfer only partially completed\n");
                        break;
                    case -EINVAL:
                        printf("    status               = -EINVAL: ISO madness, if this happens: Log off and go home\n");
                        break;
                    case -ECONNRESET:
                        printf("    status               = -ECONNRESET: URB was asynchronously unlinked by usb_unlink_urb\n");
                        break;
                    case -ESHUTDOWN:
                        printf("    status               = Device or host controller is disabled due to a problem that couldnt worked around, e.g. physical disconnect\n");
                        break;
                    default:
                        printf("    status               = UNKNOWN  : error code is not known\n");
                        break;
                }
            } else {
                printf("    status               = 0 (OK)\n");
            }
            fflush(stdout);
            printf("    flags                = %d (%s %s)\n",urb.flags,(urb.flags&USBDEVFS_URB_SHORT_NOT_OK?"USBDEVFS_URB_SHORT_NOT_OK":""),(urb.flags&USBDEVFS_URB_ISO_ASAP?"USBDEVFS_URB_ISO_ASAP":""));
            fflush(stdout);
            printf("    buffer_length        = %d\n",urb.buffer_length);
            fflush(stdout);
            printf("    actual_length        = %d\n",urb.actual_length);
            fflush(stdout);
            if (dumpData){
                printf("    BUFFER:\n");
                fflush(stdout);
                printf("      ");
                fflush(stdout);
                for (int ii=0; ii<buffer_length; ii++ ){
                    unsigned char *cp = urb.buffer;
                    printf("0x%02x ",cp[ii]);
                    if (ii%16==15)
                        printf("\n      ");
                    fflush(stdout);
                }
                printf("\n");
                fflush(stdout);
            } else {
                printf("    BUFFER: (not shown)\n");
                fflush(stdout);
            }
            printf("    start_frame          = %d\n",urb.start_frame);
            fflush(stdout);
            printf("    number_of_packets    = %d\n",urb.number_of_packets);
            fflush(stdout);
            printf("    error_count          = %d\n",urb.error_count);
            fflush(stdout);
            printf("    signr                = %d\n",urb.signr);
            fflush(stdout);
#if NO_POINTERS==0
            printf("    usercontext          = %p\n",urb.usercontext);
#else
            printf("    usercontext          = 0x--------\n");
#endif
            fflush(stdout);
	        if (urb.type==USBDEVFS_URB_TYPE_ISO){
	        	printf("    iso_frame_desc[0].length:        %d\n", urb.iso_frame_desc[0].length);
	        	printf("    iso_frame_desc[0].actual_length: %d\n", urb.iso_frame_desc[0].actual_length);
	        	printf("    iso_frame_desc[0].status:        %d\n", urb.iso_frame_desc[0].status);
	        }
            fflush(stdout);

            if (USBDEVFS_URB_TYPE_CONTROL==urb.type){
                struct usbdevfs_ctrltransfer txf;
                memcpy(&txf,urb.buffer,urb.buffer_length);
                printf("Control Packet Contents:\n");
                printf("        bRequestType     = (0x%02x) : %s | ", txf.bRequestType, ((txf.bRequestType&USB_ENDPOINT_DIR_MASK)==USB_DIR_IN?"USB_DIR_IN":"USB_DIR_OUT"));
                switch (txf.bRequestType&USB_TYPE_MASK){
                    case USB_TYPE_STANDARD: printf("USB_TYPE_STANDARD | "); break;
                    case USB_TYPE_CLASS:    printf("USB_TYPE_CLASS | "); break;
                    case USB_TYPE_VENDOR:   printf("USB_TYPE_VENDOR | "); break;
                    case USB_TYPE_RESERVED: printf("USB_TYPE_RESERVED | "); break;
                }
                switch (txf.bRequestType&USB_RECIP_MASK){
                    case USB_RECIP_DEVICE:    printf("USB_RECIP_DEVICE\n"); break;
                    case USB_RECIP_INTERFACE: printf("USB_RECIP_INTERFACE\n"); break;
                    case USB_RECIP_ENDPOINT:  printf("USB_RECIP_ENDPOINT\n"); break;
                    case USB_RECIP_OTHER:     printf("USB_RECIP_OTHER\n"); break;
                }
                printf("        bRequest         = ");
                switch (txf.bRequest){
                    case USB_REQ_GET_STATUS:        printf("USB_REQ_GET_STATUS\n"); break;
                    case USB_REQ_CLEAR_FEATURE:     printf("USB_REQ_CLEAR_FEATURE\n"); break;
                    case USB_REQ_SET_FEATURE:       printf("USB_REQ_SET_FEATURE\n"); break;
                    case USB_REQ_SET_ADDRESS:       printf("USB_REQ_SET_ADDRESS\n"); break;
                    case USB_REQ_GET_DESCRIPTOR:    printf("USB_REQ_GET_DESCRIPTOR\n"); break;
                    case USB_REQ_SET_DESCRIPTOR:    printf("USB_REQ_SET_DESCRIPTOR\n"); break;
                    case USB_REQ_GET_CONFIGURATION: printf("USB_REQ_GET_CONFIGURATION\n"); break;
                    case USB_REQ_SET_CONFIGURATION: printf("USB_REQ_SET_CONFIGURATION\n"); break;
                    case USB_REQ_GET_INTERFACE:     printf("USB_REQ_GET_INTERFACE\n"); break;
                    case USB_REQ_SET_INTERFACE:     printf("USB_REQ_SET_INTERFACE\n"); break;
                    case USB_REQ_SYNCH_FRAME:       printf("USB_REQ_SYNCH_FRAME\n"); break;
                    default: printf("%d\n", txf.bRequest); break;
                }
                printf("        wValue           = 0x%04x\n", txf.wValue);
                printf("        wIndex           = 0x%04x\n", txf.wIndex);
                printf("        wLength          = %d\n", txf.wLength);
                printf("        timeout          = %d\n", txf.timeout);
#if NO_POINTERS==0
                printf("        data             = %p", txf.data);
#else
                printf("        data             = 0x--------");
#endif
                fflush(stdout);
            }
            break;

        default:
            printf("UNKNOWN %d\n",urb.type);
            buffer_length = 100;
            break;
    }
}

