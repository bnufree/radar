#-------------------------------------------------
#
# Project created by QtCreator 2020-09-29T09:41:46
#
#-------------------------------------------------
include(../library.pri)

unix {
    DESTDIR = $${ZCHX_3RD_PATH}/common/unix/lib
} else {
    DESTDIR = $${ZCHX_3RD_PATH}/common/win/lib
}

include($$ZCHX_3RD_PATH/protobuf/protobuf.pri)

QT       += gui network

TARGET = zchx_common$${DLL_EXT}
TEMPLATE = lib

DEFINES += COMMON_LIBRARY

SOURCES += \
    zchxdatadef.cpp \
    zchxcommonutils.cpp

HEADERS +=\
    zchxdatadef.h \
    zchxcommon_global.h \
    zchxbasedata.h \
    zchxcommonutils.h \
    ZCHXRadarDataDef.pb.h \
    ZCHXAISVessel.pb.h

unix{
SOURCES += ZCHXAISVessel.pb.unix.cc \
           ZCHXRadarDataDef.pb.unix.cc

HEADERS += ZCHXAISVessel.pb.unix.h \
            ZCHXRadarDataDef.pb.unix.h
} else {
SOURCES += ZCHXAISVessel.pb.win.cc \
           ZCHXRadarDataDef.pb.win.cc

HEADERS += ZCHXAISVessel.pb.win.h \
            ZCHXRadarDataDef.pb.win.h
}

unix {
    DEST_DIR_ROOT = $${ZCHX_3RD_PATH}/common/unix
#    MyLIB.path = $${DEST_DIR_ROOT}/lib/
#    MyLIB.files += $$DESTDIR/lib$${TARGET}.so.1
#    INSTALLS += MyLIB

    MyInclude.path = $${DEST_DIR_ROOT}/include/
    MyInclude.files = $$PWD/*.h $$PWD/*.hpp
    INSTALLS += MyInclude
} else {
    DEST_DIR_ROOT = $${ZCHX_3RD_PATH}/common/win
#    MyLIB.path = $${DEST_DIR_ROOT}/lib/
#    MyLIB.files += $$DESTDIR/lib$${TARGET}.dll.a
#    INSTALLS += MyLIB

#    MyDLL.path = $${DEST_DIR_ROOT}/bin/$$CONFIG_NAME
#    MyDLL.files += $$DESTDIR/$${TARGET}.dll
#    INSTALLS += MyDLL

    MyInclude.path = $${DEST_DIR_ROOT}/include/
    MyInclude.files = $$PWD/*.h $$PWD/*.hpp
    INSTALLS += MyInclude
}
