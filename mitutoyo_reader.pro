QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        mitutoyohelper.cpp \
        usbhelper.cpp

HEADERS += \
    mitutoyohelper.h \
    usbhelper.h

contains(QMAKESPEC,.*linux-rasp-pi\d*-.*){
    message(rpi)
    CONFIG += rpi
}

unix:rpi:{
message(LIBS added for raspberry_pi)
#LIBS += -L/home/anti/raspi/sysroot/usr/lib -lraspicam -lraspicam_cv
#LIBS += -L/home/anti/raspi/sysroot/usr/lib/lib -lopencv_dnn -lopencv_gapi -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_videoio -lopencv_imgcodecs -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_imgproc -lopencv_core
LIBS += -L/home/anti/pizero/sysroot/usr/lib/arm-linux-gnueabihf/ -lusb-1.0
LIBS += -L/home/anti/pizero/sysroot/opt/vc/lib/ -lmmal -lmmal_core -lmmal_util -lmmal_vc_client -lmmal_components -lvchiq_arm -lvcsm -lcontainers -lvcos -lbcm_host
#INCLUDEPATH += /home/anti/raspi/sysroot/usr/include/raspicam
}
# LIBS += -lusb-1.0

#INCLUDEPATH +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:rpi: target.path = /home/pi/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


