#-------------------------------------------------
#
# Project created by QtCreator 2014-02-12T08:48:05
#
#-------------------------------------------------

QT       -= core
QT       -= gui

TARGET = isotope
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../src/boost.pri)

LIBS += -L../../lib/qtplatz -l$$qtLibraryTarget(adcontrols)
INCLUDEPATH += ../../src/libs

SOURCES += main.cpp

HEADERS += 
