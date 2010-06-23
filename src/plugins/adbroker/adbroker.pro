#-------------------------------------------------
#
# Project created by QtCreator 2010-06-23T10:06:16
#
#-------------------------------------------------

QT       += xml

QT       -= gui

TARGET = adbroker
TEMPLATE = lib

DEFINES += ADBROKER_LIBRARY

SOURCES += adbrokerplugin.cpp

HEADERS += adbrokerplugin.h\
        adbroker_global.h

OTHER_FILES += \
    adbroker.pluginspec
