#-------------------------------------------------
#
# Project created by QtCreator 2010-07-04T06:28:41
#
#-------------------------------------------------

QT       -= core gui

TARGET = adportable
TEMPLATE = lib
CONFIG += staticlib
include(../../qtplatz_library.pri)
include(../../boost.pri)
include(../acewrapper/acewrapper_dependencies.pri)

SOURCES += adportable.cpp \
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

HEADERS += adportable.hpp \
    array_wrapper.hpp \
    binary_search.hpp \
    configloader.hpp \
    configuration.hpp \
    constants.hpp \
    ConvertUTF.h \
    debug.hpp \
    differential.hpp \
    fft.hpp \
    float.hpp \
    is_equal.hpp \
    lifecycle_frame.hpp \
    moment.hpp \
    polfit.hpp \
    posix_path.hpp \
    protocollifecycle.hpp \
    safearray.hpp \
    spectrum_processor.hpp \
    string.hpp \
    utf.hpp
