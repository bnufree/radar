#-------------------------------------------------
#
# Project created by QtCreator 2020-12-07T13:33:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simulator
TEMPLATE = app


SOURCES += main.cpp\
        simulatorwindow.cpp \
    up_video_pthread.cpp

HEADERS  += simulatorwindow.h \
    up_video_pthread.h

FORMS    += simulatorwindow.ui
