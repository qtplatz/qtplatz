#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:23:07
#
#-------------------------------------------------

QT       -= core gui

TARGET = acewrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../adilibrary.pri)
include(../../boost.pri)
include(acewrapper_dependencies.pri)

SOURCES += acewrapper.cpp \
    timeval.cpp \
    mcastserver.cpp \
    ace_string.cpp \
    reactorthread.cpp \
    eventhandler.cpp \
    mcasthandler.cpp \
    dgramhandler.cpp \
    dgram_recv.cpp \
    timerhandler.cpp

HEADERS += acewrapper.h \
    mutex.hpp \
    timeval.h \
    mcastserver.h \
    callback.h \
    ace_string.h \
    reactorthread.h \
    eventhandler.h \
    mcasthandler.h \
    dgramhandler.h \
    dgram_recv.h \
    timerhandler.h

OTHER_FILES += \
    acewrapper.pri \
    acewrapper_dependencies.pri
