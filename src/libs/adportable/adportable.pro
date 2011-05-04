#-------------------------------------------------
#
# Project created by QtCreator 2010-07-04T06:28:41
#
#-------------------------------------------------

QT       -= core gui

TARGET = adportable
TEMPLATE = lib
CONFIG += staticlib
include(../../qtplatz_lib_static.pri)
include(../../boost.pri)
INCLUDEPATH *= ../../libs

SOURCES += adportable.cpp \
    cache.cpp \
    configloader.cpp \
    configuration.cpp \
    constants.cpp \
    ConvertUTF.c \
    debug.cpp \
    fft.cpp \
    lifecycle_frame.cpp \
    polfit.cpp \
    posix_path.cpp \
    protocollifecycle.cpp \
    spectrum_processor.cpp \
    string.cpp \
    utf.cpp

HEADERS += adportable.h \
    array_wrapper.hpp \
    binary_search.hpp \
    cache.h \
    configloader.h \
    configuration.h \
    constants.h \
    ConvertUTF.h \
    debug.h \
    differential.hpp \
    fft.h \
    float.hpp \
    is_equal.h \
    lifecycle_frame.h \
    moment.hpp \
    polfit.h \
    posix_path.h \
    protocollifecycle.h \
    safearray.hpp \
    spectrum_processor.h \
    string.h \
    utf.h
