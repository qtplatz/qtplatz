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
    adfile.cpp \
    cpio.cpp \
    fsio.cpp \
    fsio2.cpp \
    mscalibio.cpp \
    acquiredconf.cpp \
    acquireddata.cpp

HEADERS += adutils.hpp \
    processeddata.hpp \
    adfsio.hpp \
    adfile.hpp \
    cpio.hpp \
    fsio.hpp \
    fsio2.hpp \
    mscalibio.hpp \
    acquiredconf.hpp \
    acquireddata.hpp
