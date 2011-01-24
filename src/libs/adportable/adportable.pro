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
INCLUDEPATH *= ../../libs

SOURCES += adportable.cpp \
    protocollifecycle.cpp \
    lifecycle_frame.cpp \
    ConvertUTF.c \
    utf.cpp \
#    configuration.cpp \
    string.cpp \
    spectrum_processor.cpp \
    cache.cpp \
    polfit.cpp

HEADERS += adportable.h \
    binary_search.hpp \
    array_wrapper.hpp \
    safearray.hpp \
    protocollifecycle.h \
    lifecycle_frame.h \
    ConvertUTF.h \
    utf.h \
    string.h \
    configuration.h \
    spectrum_processor.h \
    cache.h \
    float.hpp \
    polfit.h
