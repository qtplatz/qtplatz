#-------------------------------------------------
#
# Project created by QtCreator 2010-07-04T09:16:29
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = test_serializer
CONFIG   += console
CONFIG   -= app_bundle

include(../../qtplatz.pri)
include(../../src/boost.pri)
include(../../src/rpath.pri)

TEMPLATE = app

LIBS += -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adinterface) 

!win32 {
  LIBS += -lboost_system \
          -lboost_filesystem \
          -lboost_iostreams \
          -lboost_date_time \
          -lboost_iostreams \
          -lboost_wserialization \
          -lbz2
}

macx {
   LIBS += -L../../bin/qtplatz.app/Contents/PlugIns
   QMAKE_LFLAGS += -Wl,-rpath,@executable_path/../../bin/qtplatz.app/Contents/PlugIns
} else {
   LIBS += -L../../lib/qtplatz
}

INCLUDEPATH += ../../src/libs

SOURCES += main.cpp
