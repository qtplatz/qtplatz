#-------------------------------------------------
#
# Project created by QtCreator 2012-09-13T13:06:58
#
#-------------------------------------------------

QT       += core
QT       -= gui
win32 {
  DEFINES += _WIN32_WINNT=0x0700
}

include(../../src/boost.pri)
include(../../src/openssl.pri)
TARGET = chemspider
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
