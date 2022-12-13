#ifndef USBHELPER_H
#define USBHELPER_H
#include <QDebug>
#include <libusb-1.0/libusb.h>

class UsbHelper
{
private:
    libusb_context* _context;
public:
    UsbHelper();
    ~UsbHelper();

    QList<libusb_device*> FindDevices(qint16 vendor, qint16 product);
};

#endif // USBHELPER_H
