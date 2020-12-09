include(../library.pri)
CONFIG += c++11 c++14
DESTDIR = $${IDE_APP_PATH}
message($$DESTDIR)
include($$ZCHX_3RD_PATH/zeromq/zmq.pri)
include($$ZCHX_3RD_PATH/protobuf/protobuf.pri)
include($$ZCHX_3RD_PATH/opencv/opencv.pri)
include($$ZCHX_3RD_PATH/common/zchx_common.pri)

QT += core positioning gui network gui-private

mingw{
    LIBS += -lws2_32 -lpsapi
    DEFINES *= Q_OS_WIN
} else {
    DEFINES -= Q_OS_WIN
}

TARGET = radar_server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    mainprocess.cpp \
    zchxcfgmgr.cpp \
    dataout/zchxdataoutputservermgr.cpp \
    dataout/zchxdataoutputserverthread.cpp \
    dataout/zchxdatathread.cpp \
    dataout/zchxmsgserverthread.cpp \
    dataout/zmqmonitorthread.cpp \
    radar/zchxradarmgr.cpp \
    radar/zchxradardataserver.cpp \
    radar/zchxradarvideowinrarthread.cpp \
    radar/zchxmulticastdatasocket.cpp \
    radar/zchxradarlocaliptestthread.cpp \
    radar/zchxradardevice.cpp \
    radar/zchxradarvideoparser.cpp \
    radar/zchxfilterareafile.cpp \
    radar/zchxradarcommon.cpp \
    radar/zchxradartargettrack.cpp \
    radar/zchxradarvideoprocessor.cpp \
    radar/zchxradarrectextraction.cpp \
    radar/targetnode.cpp \
    radar/zchxradarmergethread.cpp \
    radar/zchxradaroutputdatamgr.cpp

HEADERS += \
    mainprocess.h \
    zchxcfgmgr.h \
    dataout/zchxdataoutputservermgr.h \
    dataout/zchxdataoutputserverthread.h \
    dataout/zchxdatathread.h \
    dataout/zchxmsgserverthread.h \
    dataout/zmqmonitorthread.h \
    radar/zchxradarmgr.h \
    radar/zchxradardataserver.h \
    radar/zchxradarvideowinrarthread.h \
    radar/zchxmulticastdatasocket.h \
    radar/zchxradarlocaliptestthread.h \
    radar/zchxradardevice.h \
    radar/zchxradarvideoparser.h \
    radar/BR24.hpp \
    radar/zchxfilterareafile.h \
    radar/zchxradarcommon.h \
    radar/zchxradartargettrack.h \
    radar/zchxradarvideoprocessor.h \
    radar/zchxradarrectextraction.h \
    radar/targetnode.h \
    radar/zchxradarmergethread.h \
    radar/zchxradaroutputdatamgr.h \
    radar/radarccontroldefines.h
