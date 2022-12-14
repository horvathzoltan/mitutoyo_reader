#include "mitutoyohelper.h"

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QThread>
#include <usbhelper.h>
#include <libusb-1.0/libusb.h>

/*
add a new file in /etc/udev/rules.d/usb.rules
SUBSYSTEM=="usb", MODE="0666"

https://libusb.sourceforge.io/api-1.0/libusb_api.html
#include <libusb-1.0/libusb.h>
pi:
sudo apt-get install libusb-1.0-0
sudo apt-get install libusb-1.0-0-dev
LIBS += -lusb-1.0
//http://www.mitutoyokorea.com/upload/manual/99MAM029A.pdf
*/

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    UsbHelper usbHelper;

    QList<libusb_device*> devices;
    bool ok1 = usbHelper.FindDevices(
                MitutoyoHelper::VENDOR,
                MitutoyoHelper::PRODUCT,
                &devices);

    if(!ok1){
        qDebug() << "Cannot found devices";
        return 0;
    }

    if(devices.isEmpty()){
        qDebug() << "No Mitutoyo devide found";
        return 0;
    }

    QByteArray msg;
    bool ok = usbHelper.MitutoyoRead(devices[0], &msg, 1000, 0, 128);
    if(!ok){
        qDebug() << "Cannot read device";
        return 0;
    }

    MitutoyoHelper::Response r = MitutoyoHelper::Parse(msg);
    if(r.errorCode==MitutoyoHelper::ErrorCode::Success){
        qDebug() << "m: " << r.data << endl;
    } else{
        qDebug() << "err: " << MitutoyoHelper::ErrorCodeToString(r.errorCode) << endl;
    }

    return 0;
}


