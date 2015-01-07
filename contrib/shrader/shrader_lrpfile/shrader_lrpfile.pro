#-------------------------------------------------
#
# Project created by QtCreator 2014-04-04T09:26:47
#
#-------------------------------------------------

QT       -= gui

TARGET = shrader_lrpfile
TEMPLATE = lib

DEFINES += SHRADER_LRPFILE_LIBRARY

include(../../contrib.pri)
include(../../../src/boost.pri)
include(../../../src/adplugin.pri)

INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src

LIBS += -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adcontrols ) \
        -l$$qtLibraryTarget( portfolio ) \
        -l$$qtLibraryTarget( lrpfile )

!win32 {
    LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}

SOURCES += shrader_lrpfile.cpp \
           datafile_factory.cpp \
           datafile.cpp

HEADERS += shrader_lrpfile_global.hpp \
           datafile_factory.hpp \
           datafile.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
