#-------------------------------------------------
#
# Project created by QtCreator 2010-07-04T06:28:41
#
#-------------------------------------------------

QT       -= core gui

TARGET = adportable
TEMPLATE = lib
CONFIG += staticlib

include(../../adilibrary.pri)
include(../../boost.pri)

SOURCES += adportable.cpp \
    protocollifecycle.cpp \
    lifecycle_frame.cpp

HEADERS += adportable.h \
    binary_search.hpp \
    array_wrapper.hpp \
    safearray.hpp \
    protocollifecycle.h \
    lifecycle_frame.h
