#-------------------------------------------------
#
# Project created by QtCreator 2011-12-15T06:53:39
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = bcast
CONFIG   += console
CONFIG   -= app_bundle

!win32 {
  LIBS *= -lboost_system -lboost_date_time
}

include(../../../src/boost.pri)

INCLUDEPATH += ../../../src/libs

TEMPLATE = app


SOURCES += main.cpp \
    bcast_server.cpp \
    lifecycle.cpp

HEADERS += \
    bcast_server.hpp \
    lifecycle.hpp




