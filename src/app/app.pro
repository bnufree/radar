include(../library.pri)
CONFIG += c++11 c++14
TARGET = radar_client
TEMPLATE = app
DESTDIR = $${IDE_APP_PATH}

include($$ZCHX_3RD_PATH/ZeroMQ/zmq.pri)
include($$ZCHX_3RD_PATH/ProtoBuf/protobuf.pri)
include($$ZCHX_3RD_PATH/common/zchx_common.pri)
include($$ZCHX_3RD_PATH/zchx_ecdis/zchx_ecdis.pri)

CONFIG(release, debug|release) {
    DEFINES *= NDEBUG
}

message($$INCLUDEPATH)

DEFINES += WIN32_LEAN_AND_MEAN
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

FORMS += \
    dialog_help.ui \
    dialog_log.ui \
    radardataoutputsettings.ui \
    zchxmainwindow.ui \
    zchxradaroptwidget.ui \
    zchxlogindlg.ui \
    zchxradarbasesettingdlg.ui \
    zchxradarchannelsettingdlg.ui \
    zchxradarchannelreportctrldlg.ui \
    zchxradarparsesettingdlg.ui \
    zchxradarsettingswidget.ui \
    zchxradarfilterareasettingdlg.ui \
    zchxradarui.ui \
    zchxradarwidget.ui \
    zchxvideotermcolorsettingwidget.ui

DISTFILES += \
    app.rc

HEADERS += \
    dialog_help.h \
    dialog_log.h \
    radardataoutputsettings.h \
    zchxmainwindow.h \
    zchxradarctrlbtn.h \
    zchxradaroptwidget.h \
    network/zchxrequestworker.h \
    network/zchxradardatachange.h \
    profiles.h \
    zchxlogindlg.h \
    network/zchxrecvutils.h \
    watchdogthread.h \
    zchxradarbasesettingdlg.h \
    zchxradarchannelsettingdlg.h \
    zchxradarchannelreportctrldlg.h \
    zchxradarparsesettingdlg.h \
    zchxradarsettingswidget.h \
    zchxradarfilterareasettingdlg.h \
    zchxradarui.h \
    zchxradarwidget.h \
    zchxvideotermcolorsettingwidget.h \
    zchxvideocolorsettingwidget.h \
    zchxregistorchecker.h \
    zchxvalidationthread.h

SOURCES += \
    dialog_help.cpp \
    dialog_log.cpp \
    radardataoutputsettings.cpp \
    zchxmainwindow.cpp \
    zchxradarctrlbtn.cpp \
    zchxradaroptwidget.cpp \
    main.cpp \
    network/zchxrequestworker.cpp \
    network/zchxradardatachange.cpp \
    profiles.cpp \
    zchxlogindlg.cpp \
    network/zchxrecvutils.cpp \
    watchdogthread.cpp \
    zchxradarbasesettingdlg.cpp \
    zchxradarchannelsettingdlg.cpp \
    zchxradarchannelreportctrldlg.cpp \
    zchxradarparsesettingdlg.cpp \
    zchxradarsettingswidget.cpp \
    zchxradarfilterareasettingdlg.cpp \
    zchxradarui.cpp \
    zchxradarwidget.cpp \
    zchxvideotermcolorsettingwidget.cpp \
    zchxvideocolorsettingwidget.cpp \
    zchxregistorchecker.cpp \
    zchxvalidationthread.cpp
