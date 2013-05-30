#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T12:50:00
#
#-------------------------------------------------

QT       -= core gui

include(../../contrib.pri)
include(../../../src/boost.pri)
include(../../../src/qtplatz_servant.pri)

PROVIDER = MS-Cheminfomatics

TEMPLATE = lib
TARGET = fticr
TARGET = $$qtLibraryTarget($$TARGET)


INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src

DEFINES += FTICR_LIBRARY

SOURCES += fticr.cpp \
    datafile_factory.cpp \
    datafile.cpp \
    jcampdxparser.cpp

HEADERS += fticr.hpp\
        fticr_global.hpp \
    datafile_factory.hpp \
    datafile.hpp \
    jcampdxparser.hpp

