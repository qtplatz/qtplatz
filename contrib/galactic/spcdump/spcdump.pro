#-------------------------------------------------
#
# Project created by QtCreator 2014-04-02T07:41:48
#
#-------------------------------------------------

QT       += core
QT       -= gui
include(../../../src/boost.pri)

TARGET = spcdump
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
INCLUDEPATH += ../spcfile

!win32 {
  LIBS += -lboost_system -lboost_filesystem
}

SOURCES += main.cpp
