#-------------------------------------------------
#
# Project created by QtCreator 2013-09-27T06:52:08
#
#-------------------------------------------------

QT       -= core
QT       -= gui

TARGET = sdfile_parser
CONFIG   += console
CONFIG   -= app_bundle
macx{
  QMAKE_CXXFLAGS += -Wno-unused-parameter
}

TEMPLATE = app

include(../../src/boost.pri)

!win32: LIBS += -lboost_system

SOURCES += sdfile_parser.cpp

