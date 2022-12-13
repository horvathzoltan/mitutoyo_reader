#include "usbhelper.h"

UsbHelper::UsbHelper(){
    int rc = 0;
    rc = libusb_init(&_context);
    if(rc!=LIBUSB_SUCCESS){
        qDebug() << "LIBUSB_ERROR";
        _context = nullptr;
    }
}

UsbHelper::~UsbHelper()
{
    if(_context){
        libusb_exit(_context);
    }
}



QList<libusb_device*> UsbHelper::FindDevices(qint16 vendor, qint16 product )
{
    if(!_context)
        return {};
    libusb_device **list = nullptr;

    int rc = 0;
    ssize_t count = 0;

    count = libusb_get_device_list(_context, &list);
    if(count<LIBUSB_SUCCESS){
        qDebug() << libusb_error(count);
        return {};
    }

    //int devIx = -1;
    QList<libusb_device*> deviceList;

    for (ssize_t i = 0; i < count; ++i)
    {
        libusb_device *device = list[i];
        libusb_device_descriptor desc = {};

        rc = libusb_get_device_descriptor(device, &desc);
        if(rc!=LIBUSB_SUCCESS){
            qDebug() << libusb_error(rc);
            return {};
        }

        QString msg = QStringLiteral("Vendor:Device = %1:%2\n").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16,QChar('0'));

        if(desc.idVendor==0x0fe7 && desc.idProduct==0x4001)
        //if(desc.idVendor==vendor && desc.idProduct==product)
        {
            //devIx = static_cast<int>(i);
            msg+=QStringLiteral(" <- Mitutoyo");
            deviceList.append(device);
        }

        qDebug() << msg;
    }
    return deviceList;
}

