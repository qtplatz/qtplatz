#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:23:07
#
#-------------------------------------------------

QT       -= core gui

TARGET = acewrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../adilibrary.pri)
include(acewrapper_dependencies.pri)

SOURCES += acewrapper.cpp \
    timeval.cpp

HEADERS += acewrapper.h \
    mutex.hpp \
    timeval.h

OTHER_FILES += \
    acewrapper.pri \
    acewrapper_dependencies.pri
