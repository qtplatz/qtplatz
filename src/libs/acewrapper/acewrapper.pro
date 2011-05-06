#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:23:07
#
#-------------------------------------------------

QT       -= core gui

TARGET = acewrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatz_lib_static.pri)
include(../../boost.pri)
include(acewrapper_dependencies.pri)

INCLUDEPATH *= $$OUT_PWD/..

SOURCES += acewrapper.cpp \
    ace_string.cpp \
    brokerhelper.cpp \
    constants.cpp \
    dgramhandler.cpp \
    input_buffer.cpp \
    inputcdr.cpp \
    lifecycle_frame_serializer.cpp \
    mcasthandler.cpp \
    mcastserver.cpp \
    messageblock.cpp \
    outputcdr.cpp \
    reactorthread.cpp \
    timerhandler.cpp \
    timeval.cpp

HEADERS += acewrapper.h \
    ace_string.h \
    brokerhelper.h \
    callback.h \
    constants.h \
    dgramhandler.h \
    eventhandler.h \
    input_buffer.h \
    inputcdr.h \
    lifecycle_frame_serializer.h \
    mcasthandler.h \
    mcastserver.h \
    messageblock.h \
    mutex.hpp \
    orbservant.h \
    outputcdr.h \
    reactorthread.h \
    serialization_inet_addr.h \
    timerhandler.h \
    timeval.h

OTHER_FILES += \
    acewrapper.pri \
    acewrapper_dependencies.pri
