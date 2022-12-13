#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QThread>
#include <usbhelper.h>
#include <libusb-1.0/libusb.h>

/*
add a new file in /etc/udev/rules.d named usb.rules
SUBSYSTEM=="usb", MODE="0666"

https://libusb.sourceforge.io/api-1.0/libusb_api.html
#include <libusb-1.0/libusb.h>
sudo apt-get install libusb-1.0-0-dev
LIBS += -lusb-1.0
*/

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)
    //QCoreApplication a(argc, argv); // nem kell eventloop

//    libusb_context *context = nullptr;
//    libusb_device **list = nullptr;

//    int rc = 0;
//    ssize_t count = 0;

//    rc = libusb_init(&context);
//    if(rc!=LIBUSB_SUCCESS){
//        qDebug() << "LIBUSB_ERROR";
//        return 1;
//    }

//    count = libusb_get_device_list(context, &list);
//    if(count<LIBUSB_SUCCESS){
//        qDebug() << libusb_error(count);
//        return 1;
//    }

//    int devIx = -1;
//    for (ssize_t i = 0; i < count; ++i)
//    {
//        libusb_device *device = list[i];
//        libusb_device_descriptor desc = {};

//        rc = libusb_get_device_descriptor(device, &desc);
//        if(rc!=LIBUSB_SUCCESS){
//            qDebug() << libusb_error(rc);
//            return 1;
//        }

//        QString msg = QStringLiteral("Vendor:Device = %1:%2\n").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16,QChar('0'));

//        if(desc.idVendor==0x0fe7 && desc.idProduct==0x4001)
//        {
//            devIx = static_cast<int>(i);
//            msg+=QStringLiteral(" <- Mitutoyo");
//        }

//        qDebug() << msg;
//    }
    UsbHelper usbHelper;

    QList<libusb_device*> devices = usbHelper.FindDevices(0x0fe7,0x4001);
    if(devices.isEmpty()){
         qDebug() << "No Mitutoyo devide found";
        return 0;
    }

    //if(devIx>=0){
        libusb_device *device = devices[0];
        libusb_device_handle *handle = nullptr;

        int rc = 0;

        //handle = libusb_open_device_with_vid_pid(context, 0x0fe7, 0x4001);
        rc = libusb_open(device, &handle);
        if(rc!=LIBUSB_SUCCESS){
            qDebug() << libusb_error(rc);
            return 1;
        }

        rc = libusb_reset_device(handle);
        if(rc!=LIBUSB_SUCCESS){
            qDebug() << libusb_error(rc);
            return 1;
        }

        libusb_set_configuration(handle, 1);
        if(rc!=LIBUSB_SUCCESS){
            qDebug() << libusb_error(rc);
            return 1;
        }

        bool prevAttachedToKernel;
        rc = libusb_kernel_driver_active(handle, 0);
        if(rc==0){
            prevAttachedToKernel = false;
        }
        else if(rc==1){
             prevAttachedToKernel = true;
        }
        else{
            qDebug() << libusb_error(rc);
            return 1;
        }

        bool isDetachedToKernel;
        if(prevAttachedToKernel){
           rc = libusb_detach_kernel_driver(handle, 0); //detach it
           if(rc!=LIBUSB_SUCCESS){
               qDebug() << libusb_error(rc);
               return 1;
           }
           isDetachedToKernel=true;
       }

        rc = libusb_claim_interface(handle, 0); //claim interface 0 (the first) of device (desired device FX3 has only 1)
        if(rc!=LIBUSB_SUCCESS){
            qDebug() << libusb_error(rc);
            if(prevAttachedToKernel && isDetachedToKernel){
                rc = libusb_attach_kernel_driver(handle, 0); //attach it
            }
            return 1;
        }

        //0x40;// # Vendor Host-to-Device
        uint8_t bmRequestType=
                LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_DEVICE;
        uint8_t bRequest=0x01;
        uint16_t wValue=0xA5A5;
        uint16_t wIndex=0;
        unsigned int timeout = 100;

        rc = libusb_control_transfer(handle, bmRequestType, bRequest,
                                wValue, wIndex,
                                nullptr, 0, timeout);
        //CheckTransferError(rc);

        qDebug() << "libusb_control_transfer 0x40 ok";

        bmRequestType=//0xc0;# Vendor Device-to-Host
                LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_DEVICE;
        bRequest=0x02;
        wValue=0;libusb_release_interface(handle, 0);
        wIndex=0;

        rc = libusb_control_transfer(handle, bmRequestType, bRequest,
                                wValue, wIndex,
                                     nullptr, 0, timeout);
        //CheckTransferError(rc);

        qDebug() << "libusb_control_transfer 0xc0 ok";

        bmRequestType=
                LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_DEVICE;
        bRequest=0x03;
        wValue=0;
        wIndex=0;
        unsigned char data[3] = {'1','\r', '\0'};
        uint16_t wLength = 2;

        rc = libusb_control_transfer(handle, bmRequestType, bRequest,
                                wValue, wIndex,
                                data, wLength, timeout);
        //CheckTransferError(rc);

        qDebug() << "libusb_control_transfer 0x40 ok";
        unsigned char* DataIn = new unsigned char[512]; //data to read
        constexpr auto IN_ENDPOINT_ID = 1;
        int BytesRead, counter = 0;
        QString m;
        while (libusb_bulk_transfer(handle, (IN_ENDPOINT_ID | LIBUSB_ENDPOINT_IN), DataIn, sizeof(DataIn), &BytesRead, 0) == 0 && counter++ < 5)
        {
            char* DataIn2 = static_cast<char*>(static_cast<void *>(DataIn));
            QString msg = QString::fromLocal8Bit(DataIn2, BytesRead);
            m+=msg;
            if(msg.endsWith('\r')) break;
        }

        qDebug() << "m: " << m << Qt::endl;

        libusb_release_interface(handle, 0);
        rc = libusb_reset_device(handle);
        if(prevAttachedToKernel && isDetachedToKernel){
            rc = libusb_attach_kernel_driver(handle, 0); //attach it
        }
        libusb_close(handle);
    //}

    //int unref_devices = static_cast<int>(count);
    //libusb_free_device_list(list, unref_devices);9

    //libusb_exit(_context);
    qDebug() << "puttyom";
    return 0;
}


