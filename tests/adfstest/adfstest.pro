#-------------------------------------------------
#
# Project created by QtCreator 2011-03-13T10:52:59
#
#-------------------------------------------------

QT       += core

QT       -= gui

include(../../qtplatz.pri)
include(../../src/config.pri)
include(../../src/boost.pri)
INCLUDEPATH += ../../src/libs

macx {
  DESTDIR = $$IDE_APP_PATH/qtplatz.app/Contents/MacOS

  LIBS += -L../../bin/qtplatz.app/Contents/PlugIns
  LIBS += -Wl,-rpath ../../bin/qtplatz.app/Contents/PlugIns
  LIBS += -ladfs_debug -ladcontrols_debug -ladportable_debug
}

!win32 {
  LIBS += -lboost_system -lboost_filesystem -lboost_serialization -lboost_date_time
}


TARGET = adfstest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
