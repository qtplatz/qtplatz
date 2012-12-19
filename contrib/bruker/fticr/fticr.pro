#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T12:50:00
#
#-------------------------------------------------

QT       -= core gui

include(../../contrib.pri)
include(../../../src/boost.pri)
LIBS += -L$${QTPLATZ_BUILD_TREE}/lib/qtplatz

PROVIDER = ScienceLiaison

#TARGET = fticr
TARGET = $$qtLibraryTarget($$TARGET)
TEMPLATE = lib

INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src
DESTDIR = $$QTPLATZ_PLUGIN_PATH/$$PROVIDER

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

