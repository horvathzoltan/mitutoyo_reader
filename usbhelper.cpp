#include "usbhelper.h"

bool UsbHelper::SendConfig(libusb_device_handle* handle, const ControlPacket &p, unsigned int timeout)
{
    uint16_t wLength = 0;
    unsigned char* u = nullptr;

    if(p.data!=nullptr && !p.data.isEmpty()){
        const char* e = p.data.constData();
        const unsigned char* u1 = reinterpret_cast<const unsigned char*>(e);
        u = const_cast<unsigned char*>(u1);
        wLength = p.data.length();
    }

    int rc = libusb_control_transfer(
                handle, p.c.bmRequestType, p.c.bRequest,
                            p.c.wValue, p.c.wIndex,
                            u, wLength, timeout);
    if(rc<0){
        qDebug()<<libusb_error_name(rc);
        return false;
    }    
    return true;
}

bool UsbHelper::ReadConfig(libusb_device_handle* handle, const ControlPacket &p, unsigned int timeout, QByteArray *a)
{
    unsigned char u[10];

    int rc = libusb_control_transfer(
                handle, p.c.bmRequestType, p.c.bRequest,
                            p.c.wValue, p.c.wIndex,
                            u, p.c.wLength, timeout);
    if(rc<0){
        qDebug()<<libusb_error_name(rc);
        return false;
    }

    const char* e = reinterpret_cast<const char*>(u);

    if(a) a->setRawData(e, rc);

    //QByteArray a(e, rc);

    return true;
}

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


bool UsbHelper::FindDevices(qint16 vendor, qint16 product, QList<libusb_device*>* d )
{
    if(!_context) return false;
    libusb_device **list = nullptr;

    int rc = 0;
    ssize_t count = 0;

    count = libusb_get_device_list(_context, &list);
    if(count<LIBUSB_SUCCESS){
        qDebug() << libusb_error_name(count);
        return false;
    }

    //int devIx = -1;
    //QList<libusb_device*> deviceList;

    for (ssize_t i = 0; i < count; ++i)
    {
        libusb_device *device = list[i];
        libusb_device_descriptor desc;

        rc = libusb_get_device_descriptor(device, &desc);
        if(rc!=LIBUSB_SUCCESS){
            qDebug() << libusb_error_name(rc);
            return false;
        }

        QString msg = QStringLiteral("Vendor:Device = %1:%2\n").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16,QChar('0'));

        //if(desc.idVendor==0x0fe7 && desc.idProduct==0x4001)
        if(desc.idVendor==vendor && desc.idProduct==product)
        {
            //devIx = static_cast<int>(i);
            msg+=QStringLiteral(" <- Mitutoyo");
            if(d)d->append(device);
        }

        qDebug() << msg;
    }
    return true;
}

bool UsbHelper::MitutoyoRead(libusb_device *device, QByteArray *m, int n)
{
    if(!m) return false;
    if(n<5) return false;
    if(n>100) return false;

    libusb_device_handle *handle = nullptr;

    int rc = 0;

    //handle = libusb_open_device_with_vid_pid(context, 0x0fe7, 0x4001);
    rc = libusb_open(device, &handle);
    if(rc!=LIBUSB_SUCCESS){
        qDebug() << libusb_error_name(rc);
        return false;
    }

    rc = libusb_reset_device(handle);
    if(rc!=LIBUSB_SUCCESS){
        qDebug() << libusb_error_name(rc);
        return false;
    }

    libusb_set_configuration(handle, 1);
    if(rc!=LIBUSB_SUCCESS){
        qDebug() << libusb_error_name(rc);
        return false;
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
        qDebug() << libusb_error_name(rc);
        return false;
    }

    bool isDetachedToKernel;
    if(prevAttachedToKernel){
       rc = libusb_detach_kernel_driver(handle, 0); //detach it
       if(rc!=LIBUSB_SUCCESS){
           qDebug() << libusb_error_name(rc);
           return false;
       }
       isDetachedToKernel=true;
   }

    rc = libusb_claim_interface(handle, 0); //claim interface 0 (the first) of device (desired device FX3 has only 1)
    if(rc!=LIBUSB_SUCCESS){
        qDebug() << libusb_error_name(rc);
        if(prevAttachedToKernel && isDetachedToKernel){
            rc = libusb_attach_kernel_driver(handle, 0); //attach it
        }
        return false;
    }

    UsbHelper::ControlPacket p1 { // Vendor Host-to-Device
        .c = {
            .bmRequestType = LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_DEVICE,
            .bRequest=0x01,
            .wValue=0xA5A5,
            .wIndex=0},
                .data = {}
    };

    UsbHelper::ControlPacket p2 { // Vendor Device-to-Host
        .c = {
            .bmRequestType = LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_DEVICE,
            .bRequest=0x02,
            .wValue=0x00,
            .wIndex=0,
            .wLength=1}, //1 byteot beolvas
                .data = {}
    };

    UsbHelper::ControlPacket p3 { // Vendor Host-to-Device
        .c = {
            .bmRequestType = LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_DEVICE,
            .bRequest=0x03,
            .wValue=0x00,
            .wIndex=0},
                .data = {"1\r"}
    };

    bool ok = SendConfig(handle, p1, 200);
    if(!ok){
        qDebug() << "Cannot send package1";
        return false;
    }

    QByteArray inBytes;
    ok = ReadConfig(handle, p2, 1000, &inBytes); // beolvasás
    if(!ok){
        qDebug() << "Cannot send package2";
        return false;
    }

    ok = SendConfig(handle, p3, 200);
    if(!ok){
        qDebug() << "Cannot send package3";
        return false;
    }

    int size = 512;
    unsigned char* readBuffer = new unsigned char[size]; //data to read
    //constexpr auto IN_ENDPOINT_ID = 1;
    //unsigned char endpoint = (endpoint | LIBUSB_ENDPOINT_IN) ;
    int readBytes = 0;
    int counter = 0;
    unsigned char* p = readBuffer;
    int r = 0;

    libusb_config_descriptor *cd;
    libusb_get_active_config_descriptor(device, &cd);
    uint8_t endpoint = cd->interface->altsetting[0].endpoint[0].bEndpointAddress;

    while (counter++ < n) {
        int a = libusb_bulk_transfer(handle, endpoint, p, size, &readBytes, 200);
        if(a!=LIBUSB_SUCCESS) continue;
        if(readBytes==0) continue;
        p+=readBytes;
        r+=readBytes;
        size-=readBytes;
        if(p[readBytes-1]=='\r') break;
    }

    if(m) m->append(reinterpret_cast<char*>(readBuffer), r);
    qDebug() <<"in:" << QString(*m);

    delete[] readBuffer;
    libusb_release_interface(handle, 0);
    rc = libusb_reset_device(handle);
    if(prevAttachedToKernel && isDetachedToKernel){
        rc = libusb_attach_kernel_driver(handle, 0); //attach it
    }
    libusb_close(handle);
    return true;
}




