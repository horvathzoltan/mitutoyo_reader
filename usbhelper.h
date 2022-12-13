#ifndef USBHELPER_H
#define USBHELPER_H
#include <QDebug>
#include <libusb-1.0/libusb.h>

class UsbHelper
{
public:
    struct ControlPacket{
        libusb_control_setup c;
        QByteArray data;
        //uint16_t wLength;
        //unsigned int timeout;
    };
private:
    libusb_context* _context;

    bool SendConfig(libusb_device_handle* handle, const ControlPacket& c, unsigned int timeout);
    bool ReadConfig(libusb_device_handle* handle, const ControlPacket& c, unsigned int timeout, QByteArray *a);
public:


    static const qint16 MITUTOYO_VENDOR;
    static const qint16 MITUTOYO_PRODUCT;

    UsbHelper();
    ~UsbHelper();

    bool FindDevices(qint16 vendor, qint16 product, QList<libusb_device*>* d);
    bool MitutoyoRead(libusb_device* device, QString* m);
};

#endif // USBHELPER_H
