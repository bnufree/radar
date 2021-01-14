#-------------------------------------------------
#
# Project created by QtCreator 2020-12-07T13:33:54
#
#-------------------------------------------------

include(../library.pri)
CONFIG += c++11 c++14
DESTDIR = $${IDE_APP_PATH}
include($$ZCHX_3RD_PATH/winsock/winsock.pri)

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simulator
TEMPLATE = app

SOURCES += main.cpp\
        simulatorwindow.cpp \
    zchxmulticastdatasocket.cpp \
    zchxradarvideowinrarthread.cpp \
    zchxsimulatethread.cpp \
    zchxradardatareceiver.cpp \
    zchxradardatasender.cpp

HEADERS  += simulatorwindow.h \
    zchxmulticastdatasocket.h \
    zchxradarvideowinrarthread.h \
    zchxsimulatethread.h \
    zchxradardatareceiver.h \
    zchxradardatasender.h

FORMS    += simulatorwindow.ui
