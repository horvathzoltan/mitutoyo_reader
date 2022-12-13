#include "mitutoyohelper.h"

const qint16 MitutoyoHelper::VENDOR = 0x0fe7;
const qint16 MitutoyoHelper::PRODUCT = 0x4001;

MitutoyoHelper::Response MitutoyoHelper::Parse(const QByteArray &b)
{
    Response r;
    if(b.length()<4){
        r.errorCode = ErrorCode::UnknownError;
        return r;
    }
    r.code = b[0];
    r.channel = b[1];
    if(r.code == '9'){ // error format
        int c = QString(b[2]).toInt();
        ErrorCode a = static_cast<ErrorCode>(c);
        r.errorCode=a;
    }
    else{
        r.errorCode=ErrorCode::Success;
        r.measuringItem = b[2];
        r.data = QString::fromLatin1(b.mid(3, b.length()-4));
    }
    return r;
}

QString MitutoyoHelper::ErrorCodeToString(ErrorCode r){
    switch(r){
        case ErrorCode::Success:return "Success";
        case ErrorCode::NoData:return "NoData";
        case ErrorCode::FormatError:return "FormatError";
        case ErrorCode::SystemError:return "SystemError";
        default:return "UnknownError";
    }
}

