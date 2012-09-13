#-------------------------------------------------
#
# Project created by QtCreator 2012-09-12T17:04:37
#
#-------------------------------------------------

QT       += core
QT       -= gui
win32 {
  DEFINES += _WIN32_WINNT=0X0501
} else {
}

include(../../src/boost.pri)
include(../../src/openssl.pri)

TARGET = boost_ssl_server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
