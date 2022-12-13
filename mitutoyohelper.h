#ifndef MITUTOYOHELPER_H
#define MITUTOYOHELPER_H
#include <QByteArray>
#include <QString>

class MitutoyoHelper
{
public:
    static const qint16 VENDOR;
    static const qint16 PRODUCT;

    enum ErrorCode:int{
        Success = 0,
        NoData = 1,
        FormatError =2,
        SystemError = 9,
        UnknownError = 99
    };

    static QString ErrorCodeToString(ErrorCode r);

    struct Response {
        unsigned char code;//0=ok, 9=err
        unsigned char channel;
        unsigned char measuringItem;
        ErrorCode errorCode;//1:noData
        QString data;
    };

    static Response Parse(const QByteArray& b);
};

#endif // MITUTOYOHELPER_H
