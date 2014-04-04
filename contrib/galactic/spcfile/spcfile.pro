#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T12:50:00
#
#-------------------------------------------------

QT       -= gui

include(../../contrib.pri)
include(../../../src/boost.pri)
include(../../../src/qtplatzlibrary.pri)

PROVIDER = MS-Cheminfomatics

TEMPLATE = lib
TARGET = spcfile
TARGET = $$qtLibraryTarget($$TARGET)

INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src

DEFINES += SPCFILE_LIBRARY

LIBS += -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adcontrols )

!win32 {
    LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}

SOURCES += spcfile.cpp \
           spchdr.cpp \
           subhdr.cpp

HEADERS += \
        spcfile_global.hpp \
        spchdr.hpp \
        spc_h.hpp \
        subhdr.hpp
