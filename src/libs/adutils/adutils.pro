#-------------------------------------------------
#
# Project created by QtCreator 2010-12-26T13:35:38
#
#-------------------------------------------------

QT       -= gui

TARGET = adutils
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatzstaticlib.pri)

INCLUDEPATH += ../../libs
include (../../boost.pri)

SOURCES += adutils.cpp \
    processeddata.cpp \
    adfsio.cpp \
    cpio.cpp \
    fsio.cpp \
    mscalibio.cpp \
    acquiredconf.cpp \
    acquireddata.cpp

HEADERS += adutils.hpp \
    processeddata.hpp \
    adfsio.hpp \
    cpio.hpp \
    fsio.hpp \
    mscalibio.hpp \
    acquiredconf.hpp \
    acquireddata.hpp
