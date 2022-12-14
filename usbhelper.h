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
    UsbHelper();
    ~UsbHelper();

    bool FindDevices(qint16 vendor, qint16 product, QList<libusb_device*>* d);
    bool MitutoyoRead(libusb_device* device, QByteArray* m, unsigned int n,unsigned int n1, unsigned int bufferSize);
private slots:
    void foo();
};

#endif // USBHELPER_H
