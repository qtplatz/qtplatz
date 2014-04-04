#-------------------------------------------------
#
# Project created by QtCreator 2014-04-04T09:26:47
#
#-------------------------------------------------

QT       -= gui

include(../../contrib.pri)
include(../../../src/boost.pri)
include(../../../src/adplugin.pri)

PROVIDER = MS-Cheminfomatics

TARGET = galactic_spcfile
TEMPLATE = lib

DEFINES += GALACTIC_SPCFILE_LIBRARY
INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src

LIBS += -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adcontrols ) \
        -l$$qtLibraryTarget( portfolio ) \
        -l$$qtLibraryTarget( spcfile )

!win32 {
    LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}


SOURCES += galactic_spcfile.cpp \
           datafile_factory.cpp \
           datafile.cpp

HEADERS += galactic_spcfile.hpp\
           galactic_spcfile_global.hpp \
           datafile_factory.hpp \
           datafile.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
