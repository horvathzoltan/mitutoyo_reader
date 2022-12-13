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

    UsbHelper usbHelper;

    QList<libusb_device*> devices;
    bool ok1 = usbHelper.FindDevices(
                UsbHelper::MITUTOYO_VENDOR,UsbHelper::MITUTOYO_PRODUCT,
                &devices);

    if(!ok1){
        qDebug() << "Cannot found devices";
        return 0;
    }
    if(devices.isEmpty()){
        qDebug() << "No Mitutoyo devide found";
        return 0;
    }

    QString msg;
    bool ok = usbHelper.MitutoyoRead(devices[0], &msg);
    if(!ok){
        qDebug() << "Cannot read device";
        return 0;
    }
    qDebug() << "m: " << msg << Qt::endl;
    return 0;
}


