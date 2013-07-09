#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T12:50:00
#
#-------------------------------------------------

QT       -= gui

include(../../contrib.pri)
include(../../../src/boost.pri)
include(../../../src/adplugin.pri)

PROVIDER = MS-Cheminfomatics

TEMPLATE = lib
TARGET = fticr
TARGET = $$qtLibraryTarget($$TARGET)


INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src

DEFINES += FTICR_LIBRARY

LIBS += -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adcontrols ) \
        -l$$qtLibraryTarget( portfolio )

!win32 {
    LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}

SOURCES += fticr.cpp \
    datafile_factory.cpp \
    datafile.cpp \
    jcampdxparser.cpp

HEADERS += fticr.hpp\
        fticr_global.hpp \
    datafile_factory.hpp \
    datafile.hpp \
    jcampdxparser.hpp

