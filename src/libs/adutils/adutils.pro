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
    fsio.cpp

HEADERS += adutils.hpp \
    processeddata.hpp \
    adfsio.hpp \
    fsio.hpp
