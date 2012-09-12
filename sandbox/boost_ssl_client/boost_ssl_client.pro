#-------------------------------------------------
#
# Project created by QtCreator 2012-09-12T17:05:30
#
#-------------------------------------------------

QT       += core
QT       -= gui
include(../../src/boost.pri)
include(../../src/openssl.pri)

TARGET = boost_ssl_client
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
