#-------------------------------------------------
#
# Project created by QtCreator 2010-06-28T18:00:36
#
#-------------------------------------------------

QT       -= core gui

TARGET = admachine
TEMPLATE = lib

include(../../adilibrary.pri)

DEFINES += ADMACHINE_LIBRARY

SOURCES += machinestatecontroller.cpp

HEADERS += machinestatecontroller.h\
        admachine_global.h

OTHER_FILES += \
    admachine.pri \
    admachine_dependencies.pri
