#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:23:07
#
#-------------------------------------------------

QT       -= gui

TARGET = acewrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatzstaticlib.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

win32: QMAKE_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS

INCLUDEPATH *= $$OUT_PWD/..

SOURCES += acewrapper.cpp \
    ace_string.cpp \
    brokerhelper.cpp \
    constants.cpp \
    input_buffer.cpp \
    inputcdr.cpp \
    lifecycle_frame_serializer.cpp \
    mcastserver.cpp \
    outputcdr.cpp \
    reactorthread.cpp \
    timerhandler.cpp \
    timeval.cpp \
    ifconfig.cpp \
    servantmanager.cpp \
    iorsender.cpp \
    iorquery.cpp

HEADERS += acewrapper.hpp \
    ace_string.hpp \
    brokerhelper.hpp \
    callback.hpp \
    constants.hpp \
    eventhandler.hpp \
    input_buffer.hpp \
    inputcdr.hpp \
    lifecycle_frame_serializer.hpp \
    mcastserver.hpp \
    mutex.hpp \
    orbservant.hpp \
    outputcdr.hpp \
    reactorthread.hpp \
    serialization_inet_addr.hpp \
    timerhandler.hpp \
    timeval.hpp \
    ifconfig.hpp \
    ifconfig_macosx.hpp \
    ifconfig_linux.hpp \
    servantmanager.hpp \
    iorsender.hpp \
    iorquery.hpp

OTHER_FILES += \
    acewrapper.pri



