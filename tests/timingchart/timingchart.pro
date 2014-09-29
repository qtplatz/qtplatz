#-------------------------------------------------
#
# Project created by QtCreator 2014-09-28T07:55:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../../qtplatz.pri)
include(../../src/config.pri)
include(../../src/boost.pri)
include(../../src/qwt.pri)

INCLUDEPATH += ../../src/libs

TARGET = timingchart
TEMPLATE = app
DESTDIR  = $$IDE_APP_PATH

macx {
    DESTDIR = $$IDE_APP_PATH/qtplatz.app/Contents/MacOS
    LIBS += -L../../bin/qtplatz.app/Contents/PlugIns
    LIBS += -Wl,-rpath ../../bin/qtplatz.app/Contents/PlugIns
#   LIBS += -ladfs_debug -ladcontrols_debug -ladportable_debug
}

LIBS += -L../../lib -l$$qtLibraryTarget( adplot )

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.hpp

FORMS    += mainwindow.ui
