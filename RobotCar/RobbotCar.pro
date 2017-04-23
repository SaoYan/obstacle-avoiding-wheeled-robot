#-------------------------------------------------
#
# Project created by QtCreator 2016-06-09T17:53:22
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RobbotCar
TEMPLATE = app

INCLUDEPATH += C:\OpenCV2\include

LIBS += -LC:\OpenCV2\lib\
-lopencv_core244\
-lopencv_highgui244\
-lopencv_imgproc244\
-lopencv_features2d244\
-lopencv_calib3d244\
-lopencv_video244

SOURCES += main.cpp\
        robotcar.cpp \
    colorhistogram.cpp \
    contentfinder.cpp \
    changecommands.cpp

HEADERS  += robotcar.h \
    colorhistogram.h \
    contentfinder.h \
    changecommands.h

FORMS    += robotcar.ui \
    changecommands.ui

RESOURCES += \
    myres.qrc
