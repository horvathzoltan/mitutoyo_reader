#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QThread>
#include <libusb-1.0/libusb.h>

void CheckOpenError(int err)
{
    switch(err){
    case 0: return;
    case LIBUSB_ERROR_NO_MEM:qDebug() << "memory allocation failure"; break;
    case LIBUSB_ERROR_ACCESS:qDebug() << "the user has insufficient permissions";break;
    case LIBUSB_ERROR_NO_DEVICE:qDebug() << "the device has been disconnected";break;
    default:qDebug() << "other failure";break;
    }
    throw;
}

void CheckTransferError(int l){
    if(l>=0) return; //on success, the number of bytes actually transferred
    switch(l){
    case LIBUSB_ERROR_TIMEOUT:qDebug() << "the transfer timed out"; break;
    case LIBUSB_ERROR_PIPE:qDebug() << "the control request was not supported by the device";break;
    case LIBUSB_ERROR_NO_DEVICE:qDebug() << "the device has been disconnected";break;
    case LIBUSB_ERROR_BUSY:qDebug() << "called from event handling context";break;
    case LIBUSB_ERROR_INVALID_PARAM:qDebug() << "the transfer size is larger than the operating system and/or hardware can support (see Transfer length limitations)";break;
    default:qDebug() << "other failure";break;
    }
}
//init
/*
LIBUSB_SUCCESS on success
LIBUSB_ERROR_INVALID_PARAM if the option or arguments are invalid
LIBUSB_ERROR_NOT_SUPPORTED if the option is valid but not supported on this platform
LIBUSB_ERROR_NOT_FOUND if LIBUSB_OPTION_USE_USBDK is valid on this platform but UsbDk is not available
*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    libusb_context *context = nullptr;
    libusb_device **list = nullptr;

    int rc = 0;
    ssize_t count = 0;

    rc = libusb_init(&context);
    assert(rc == 0);

    count = libusb_get_device_list(context, &list);
//    assert(count > 0);

    // d = usb.core.find(idVendor=0x0fe7, idProduct=0x4001)
    /*
d.reset()
d.set_configuration(1)
c = d.get_active_configuration()
epin = d.get_active_configuration().interfaces()[0].endpoints()[0]
bmRequestType=0x40 # Vendor Host-to-Device
bRequest=0x01
wValue=0xA5A5
wIndex=0
d.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex)

bmRequestType=0xC0 # Vendor Device-to-Host
bRequest=0x02
wValue=0
wIndex=0
length=1
res1 = d.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, length)
log.debug("Device Vendor resp: {}".format(res1))

bmRequestType=0x40 #0b01000000
bRequest=0x03
wValue=0
wIndex=0
data = b"1\r"

d.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, data)
*/
    int devIx = -1;
    for (ssize_t i = 0; i < count; ++i)
    {
        libusb_device *device = list[i];
        libusb_device_descriptor desc = {};

        rc = libusb_get_device_descriptor(device, &desc);
        assert(rc == 0);

        //printf("Vendor:Device = %04x:%04x\n", desc.idVendor, desc.idProduct);
        QString msg = QStringLiteral("Vendor:Device = %1:%2\n").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16,QChar('0'));

        if(desc.idVendor==0x0fe7 && desc.idProduct==0x4001)
        {
            devIx = static_cast<int>(i);
            msg+=QStringLiteral(" <- Mitutoyo: %1").arg(devIx);
        }

        qDebug() << msg;
    }

    if(devIx>=0){
        libusb_device *device = list[devIx];

        libusb_device_handle *handle = nullptr;

        //handle = libusb_open_device_with_vid_pid(context, 0x0fe7, 0x4001);

        rc = libusb_open(device, &handle);

        //d.reset()
        //d.set_configuration(1)
        //c = d.get_active_configuration()
        //epin = d.get_active_configuration().interfaces()[0].endpoints()[0]

        libusb_reset_device(handle);
        libusb_set_configuration(handle, 1);
        if (libusb_kernel_driver_active(handle, 0) == 1) { //find out if kernel driver is attached
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


