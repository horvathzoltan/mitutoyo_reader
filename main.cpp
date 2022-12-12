#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QThread>
#include <libusb-1.0/libusb.h>

/*
add a new file in /etc/udev/rules.d named usb.rules
SUBSYSTEM=="usb", MODE="0666"
*/

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)
    //QCoreApplication a(argc, argv); // nem kell eventloop

    libusb_context *context = nullptr;
    libusb_device **list = nullptr;

    int rc = 0;
    ssize_t count = 0;

    rc = libusb_init(&context);
    if(rc!=0){
        qDebug() << "LIBUSB_ERROR";
        return 1;
    }

    count = libusb_get_device_list(context, &list);
    if(count<LIBUSB_SUCCESS){
        qDebug() << libusb_error(count);
        return 1;
    }

    int devIx = -1;
    for (ssize_t i = 0; i < count; ++i)
    {
        libusb_device *device = list[i];
        libusb_device_descriptor desc = {};

        rc = libusb_get_device_descriptor(device, &desc);
        if(rc<LIBUSB_SUCCESS){
            qDebug() << libusb_error(count);
            return 1;
        }

        QString msg = QStringLiteral("Vendor:Device = %1:%2\n").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16,QChar('0'));

        if(desc.idVendor==0x0fe7 && desc.idProduct==0x4001)
        {
            devIx = static_cast<int>(i);
            msg+=QStringLiteral(" <- Mitutoyo");
        }

        qDebug() << msg;
    }

    if(devIx>=0){
        libusb_device *device = list[devIx];

        libusb_device_handle *handle = nullptr;

        //handle = libusb_open_device_with_vid_pid(context, 0x0fe7, 0x4001);
        rc = libusb_open(device, &handle);

        libusb_reset_device(handle);
        libusb_set_configuration(handle, 1);
        bool isAttachedToKernel = libusb_kernel_driver_active(handle, 0) == 1;
        if(isAttachedToKernel) {
               libusb_detach_kernel_driver(handle, 0); //detach it
           }
        libusb_claim_interface(handle, 0); //claim interface 0 (the first) of device (desired device FX3 has only 1)


        //CheckOpenError(rc);


        uint8_t bmRequestType=0x40;// # Vendor Host-to-Device
        uint8_t bRequest=0x01;
        uint16_t wValue=0xA5A5;
        uint16_t wIndex=0;
        unsigned char data[3] = {'\0','\0', '\0'};
        uint16_t wLength = 0;
        //unsigned int timeout = 500;

        rc = libusb_control_transfer(handle, bmRequestType, bRequest,
                                wValue, wIndex,
                                data, wLength, 100);
        //CheckTransferError(rc);

        qDebug() << "libusb_control_transfer 0x40 ok";

        bmRequestType=0xc0;// # Vendor Host-to-Device
        bRequest=0x02;
        wValue=0;
        wIndex=0;
        wLength = 0;
        //timeout = 500;

        rc = libusb_control_transfer(handle, bmRequestType, bRequest,
                                wValue, wIndex,
                                     data, wLength, 100);
        //CheckTransferError(rc);

        qDebug() << "libusb_control_transfer 0xc0 ok";

        bmRequestType=0x40; //#0b01000000
        bRequest=0x03;
        wValue=0;
        wIndex=0;
        data[0] = '1';
        data[1] = '\r';
        data[2] = '\0';
        wLength = 2;

        rc = libusb_control_transfer(handle, bmRequestType, bRequest,
                                wValue, wIndex,
                                data, wLength, 100);
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
        //libusb_release_interface(handle, 0);
        //libusb_close(handle);

    }

    //int unref_devices = static_cast<int>(count);
    //libusb_free_device_list(list, unref_devices);
    libusb_exit(context);
    qDebug() << "puttyom";
    return 0;
}


