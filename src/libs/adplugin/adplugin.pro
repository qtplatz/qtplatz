#-------------------------------------------------
#
# Project created by QtCreator 2010-08-05T16:33:48
#
#-------------------------------------------------

QT       += gui

TARGET = adplugin
TEMPLATE = lib
include(../../adilibrary.pri)

DEFINES += ADPLUGIN_LIBRARY

SOURCES += adplugin.cpp

HEADERS += adplugin.h\
        adplugin_global.h \
    imonitor.h \
    icontrolmethodeditor.h \
    ifactory.h
