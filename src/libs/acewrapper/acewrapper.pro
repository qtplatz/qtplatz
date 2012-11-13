#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:23:07
#
#-------------------------------------------------

QT       -= core gui

TARGET = acewrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatz_library.pri)
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
    timeval.cpp \
    ifconfig.cpp \
    servantmanager.cpp

HEADERS += acewrapper.hpp \
    ace_string.hpp \
    brokerhelper.hpp \
    callback.hpp \
    constants.hpp \
    dgramhandler.hpp \
    eventhandler.hpp \
    input_buffer.hpp \
    inputcdr.hpp \
    lifecycle_frame_serializer.hpp \
    mcasthandler.hpp \
    mcastserver.hpp \
    messageblock.hpp \
    mutex.hpp \
    orbservant.hpp \
    outputcdr.hpp \
    reactorthread.hpp \
    serialization_inet_addr.hpp \
    timerhandler.hpp \
    timeval.hpp \
    ifconfig.hpp \
    servantmanager.hpp

OTHER_FILES += \
    acewrapper.pri \
    acewrapper_dependencies.pri


