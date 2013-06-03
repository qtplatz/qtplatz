#-------------------------------------------------
#
# Project created by QtCreator 2012-04-21T09:34:34
#
#-------------------------------------------------

QT       -= gui
TARGET = compassxtract

include(../../contrib.pri)
include(../../../src/boost.pri)
include(../../../src/adplugin.pri)

INCLUDEPATH += .
INCLUDEPATH += "C:\Program Files (x86)\Bruker Daltonik\CompassXtract"

DEFINES += COMPASSXTRACT_LIBRARY

SOURCES += compassxtract.cpp \
    datafile_factory.cpp \
    datafile.cpp \
    safearray.cpp

HEADERS += compassxtract.hpp\
        compassxtract_global.hpp \
    datafile_factory.hpp \
    datafile.hpp \
    safearray.hpp

