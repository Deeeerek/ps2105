/*
    (C) 2009, Tiago Gasiba
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <linux/usbdevice_fs.h>
#include <asm/byteorder.h>
#include <errno.h>

#define	_SYS_IOCTL_H	1
#include <bits/ioctls.h>
#include <linux/usb/ch9.h>

#define NO_POINTERS 1
#include "dump_urb.h"


/*
// ##########################################################################################################
// #  Interpose OPEN
// ##########################################################################################################
int open(const char *pathname, int flags, int mode)
{
    static int (*real_open)(const char *pathname, int flags, int mode) = NULL;

    if (!real_open)
        real_open = dlsym(RTLD_NEXT, "open");
    int ret = real_open(pathname,flags,mode);
    printf("CAPTURE: %d = open(\"%s\", %x, %x)\n",ret,pathname,flags,mode);
    fflush(stdout);
    return ret;
}

// ##########################################################################################################
// #  Interpose CLOSE
// ##########################################################################################################
int close(int fd)
{
    static int (*real_close)(int) = NULL;

    if (!real_close)
        real_close = dlsym(RTLD_NEXT, "close");
    int ret = real_close(fd);
    printf("CAPTURE: %d = close(%d)\n",ret,fd);
    fflush(stdout);
    return ret;
}

// ##########################################################################################################
// #  Interpose WRITE
// ##########################################################################################################
int write( int fd, void *buf, size_t count )
{
    static int (*real_write)(int fd, void *buf, size_t count ) = NULL;
    int ret;

    if (!real_write)
        real_write = dlsym(RTLD_NEXT, "write");

    printf("\n      ");
    for (int ii=0; ii<count; ii++ ){
        unsigned char *cp = (unsigned char *)buf;
        printf("0x%02x ",cp[ii]);
        if (ii%16==15)
            printf("\n      ");
    }
    printf("\n");

    ret = real_write(fd,buf,count);
    #if NO_POINTERS==0
        printf("END OF CAPTURE:    %d=write(%d, %p, %d)\n",ret,fd,buf,count);
    #else
        printf("END OF CAPTURE:    %d=write(%d, 0x--------, %d)\n",ret,fd,count);
    #endif
    printf("--------------------------------------------------------------\n");
    fflush(stdout);
    return ret;
}

// ##########################################################################################################
// #  Interpose READ
// ##########################################################################################################
int read( int fd, void *buf, size_t count )
{
    static int (*real_read)(int fd, void *buf, size_t count ) = NULL;
    int ret;

    if (!real_read)
        real_read = dlsym(RTLD_NEXT, "read");

    ret = real_read(fd,buf,count);

    printf("\n      ");
    for (int ii=0; ii<ret; ii++ ){
        unsigned char *cp = (unsigned char *)buf;
        printf("0x%02x ",cp[ii]);
        if (ii%16==15)
            printf("\n      ");
    }
    printf("\n");

#if NO_POINTERS==0
        printf("END OF CAPTURE:    %d=read(%d, %p, %d)\n",ret,fd,buf,count);
#else
        printf("END OF CAPTURE:    %d=read(%d, 0x--------, %d)\n",ret,fd,count);
#endif
    printf("--------------------------------------------------------------\n");
    fflush(stdout);
    return ret;
}

*/

// ##########################################################################################################
// #  Interpose IOCTL
// ##########################################################################################################
int ioctl(int d, int request, void *pnt)
{
    int ret;
    static int (*real_ioctl)(int d, int request, void *pnt) = NULL;
    int flag_exec_ioctl = 1;

    if (!real_ioctl)
        real_ioctl= dlsym(RTLD_NEXT, "ioctl");


    /*
        USBDEVFS_CLAIMINTERFACE

        This is used to force usbfs to claim a specific interface, which has not previously
        been claimed by usbfs or any other kernel driver. The ioctl parameter is an integer
        holding the number of the interface (bInterfaceNumber from descriptor).

        Note that if your driver doesn't claim an interface before trying to use one of its
        endpoints, and no other driver has bound to it, then the interface is automatically
        claimed by usbfs.

        This claim will be released by a RELEASEINTERFACE ioctl, or by closing the file
        descriptor. File modification time is not updated by this request.
    */
    if (USBDEVFS_CLAIMINTERFACE==request){
#if NO_POINTERS==0
        printf("  USBDEVFS_CLAIMINTERFACE  %d\n",*(int*)pnt);
#else
        printf("  USBDEVFS_CLAIMINTERFACE\n");
#endif
    }

    /*
        USBDEVFS_RELEASEINTERFACE
        This is used to release the claim usbfs made on interface, either implicitly or because
        of a USBDEVFS_CLAIMINTERFACE call, before the file descriptor is closed. The ioctl
        parameter is an integer holding the number of the interface (bInterfaceNumber from
        descriptor); File modification time is not updated by this request.

        Warning
        No security check is made to ensure that the task which made the claim is the one which
        is releasing it. This means that user mode driver may interfere other ones.
    */
    if (USBDEVFS_RELEASEINTERFACE==request){
#if NO_POINTERS==0
        printf("  USBDEVFS_RELEASEINTERFACE %d\n",*(int*)pnt);
#else
        printf("  USBDEVFS_RELEASEINTERFACE\n");
#endif
    }

    /*
        USBDEVFS_RESETEP
        Resets the data toggle value for an endpoint (bulk or interrupt) to DATA0. The ioctl
        parameter is an integer endpoint number (1 to 15, as identified in the endpoint
        descriptor), with USB_DIR_IN added if the device's endpoint sends data to the host.

        Warning
        Avoid using this request. It should probably be removed. Using it typically means the
        device and driver will lose toggle synchronization. If you really lost synchronization,
        you likely need to completely handshake with the device, using a request like CLEAR_HALT
        or SET_INTERFACE.
    */
    if (USBDEVFS_RESETEP==request){
        unsigned char endPoint = *(unsigned char *)pnt;
        if ( USB_DIR_IN == (USB_DIR_IN & endPoint) )
            printf("  USBDEVFS_RESETEP  %d|USB_DIR_IN\n",endPoint^USB_DIR_IN);
        else
            printf("  USBDEVFS_RESETEP  %d|USB_DIR_OUT\n",endPoint);
    }

    /*
        USBDEVFS_SUBMITURB
    */
    if( USBDEVFS_SUBMITURB==request ){
        struct usbdevfs_urb *urb = (struct usbdevfs_urb *)pnt;
        printf("    USBDEVFS_SUBMITURB\n");
        int direction = urb->endpoint&USB_ENDPOINT_DIR_MASK;
        if (direction==USB_DIR_IN){
            flag_exec_ioctl=0;
            ret = real_ioctl(d,request,pnt);
        }
        urb = (struct usbdevfs_urb*)pnt;
        if (direction==USB_DIR_IN)
            dump_urb(*urb,0);       // do not show contents of the buffer when requesting input
                                    // this will come afterwards in another URB (USBDEVFS_REAPURBNDELAY)
        else
            dump_urb(*urb,1);       // direction OUT: show the packet sent...
        printf("\n");
    }


    /*
        USBDEVFS_RESET
        Does a USB level device reset. The ioctl parameter is ignored. After the reset, this
        rebinds all device interfaces. File modification time is not updated by this request.

        Warning
        Avoid using this call until some usbcore bugs get fixed, since it does not fully
        synchronize device, interface, and driver (not just usbfs) state.
    */
    if (USBDEVFS_RESET==request){
#if NO_POINTERS==0
        printf("  USBDEVFS_RESET %d\n",(int)pnt);
#else
        printf("  USBDEVFS_RESET\n");
#endif
    }

    /*
        USBDEVFS_SETINTERFACE
        Sets the alternate setting for an interface.
        The ioctl parameter is a pointer to a structure like this:

        struct usbdevfs_setinterface {
            unsigned int  interface;
            unsigned int  altsetting;
        }; 

        File modification time is not updated by this request.

        Those struct members are from some interface descriptor applying to the current
        configuration. The interface number is the bInterfaceNumber value, and the
        altsetting number is the bAlternateSetting value. (This resets each endpoint
        in the interface.)
    */
    if (USBDEVFS_SETINTERFACE==request) {
        struct usbdevfs_setinterface setInterface = *(struct usbdevfs_setinterface*)pnt;
        printf("  USBDEVFS_SETINTERFACE\n");
        printf("    bInterfaceNumber  = %d\n",setInterface.interface);
        printf("    bAlternateSetting = %d\n",setInterface.altsetting);
    }

    /*
        USBDEVFS_CLEAR_HALT

        Clears endpoint halt (stall) and resets the endpoint toggle. This is only
        meaningful for bulk or interrupt endpoints. The ioctl parameter is an integer
        endpoint number (1 to 15, as identified in an endpoint descriptor), masked with
        USB_DIR_IN when referring to an endpoint which sends data to the host from the
        device.

        Use this on bulk or interrupt endpoints which have stalled, returning -EPIPE
        status to a data transfer request. Do not issue the control request directly,
        since that could invalidate the host's record of the data toggle.
    */
    if (USBDEVFS_CLEAR_HALT==request) {
        unsigned char endPoint = *(unsigned char *)pnt;
        if ( USB_DIR_IN == (USB_DIR_IN & endPoint) )
            printf("  USBDEVFS_CLEAR_HALT %d|USB_DIR_IN\n",endPoint^USB_DIR_IN);
        else
            printf("  USBDEVFS_CLEAR_HALT %d|USB_DIR_OUT\n",endPoint);
    }


    /*
        NOT SURE ABOUT THIS FUNCTION!!!
    */
    if (USBDEVFS_REAPURBNDELAY==request){
        struct usbdevfs_urb *urb;
#if NO_POINTERS==0
        printf("  USBDEVFS_REAPURBNDELAY   %p\n",pnt);
#else
        printf("  USBDEVFS_REAPURBNDELAY\n");
#endif

        flag_exec_ioctl=0;

        ret = real_ioctl(d,request,pnt);


        // pnt is a pointer to a pointer!
        if ( (!ret) && (struct usbdevfs_urb**)pnt ){
            urb = *(struct usbdevfs_urb**)pnt;
            int direction = urb->endpoint&USB_ENDPOINT_DIR_MASK;

//            if (urb->buffer_length==1024){
//                unsigned char *cp = urb->buffer;
//                for (int jj=0; jj<(1024-20); jj++ )
//                    cp[20+jj] = (unsigned int) jj & 0xff;
//            }

            if (direction==USB_DIR_IN)
                dump_urb(*urb,1);        // this is the answer to an input request, therefore
                                         // we want to dump the packet
            else
                dump_urb(*urb,0);        // this is a REAPURBNDELAY with output direction
                                         // therefore we do not show the packet
        } else
            printf("ret = %d\n",ret);
        printf("\n");
    }



    /*
        NOT SURE ABOUT THIS FUNCTION!!!
    */
    if (USBDEVFS_DISCARDURB==request){
        struct usbdevfs_urb *urb;

        flag_exec_ioctl=0;
        ret = real_ioctl(d,request,pnt);

        // pnt is a pointer to an URB
        if ( (!ret) && (struct usbdevfs_urb*)pnt ){
            urb = (struct usbdevfs_urb*)pnt;
            dump_urb(*urb,1);
        } else
            printf("ret = %d\n",ret);
        printf("\n");
    }



    printf( "\n");
    if (1==flag_exec_ioctl)
        ret = real_ioctl(d,request,pnt);
#if NO_POINTERS==0
    printf("END OF CAPTURE:    %d=IOCTL(%d,0x%x,0x%x)\n",ret,d,request,pnt);
#else
    printf("END OF CAPTURE:    %d=IOCTL(%d,0x--------,0x--------)\n",ret,d);
#endif
    printf("--------------------------------------------------------------\n");

    fflush(stdout);
    return ret;
}

