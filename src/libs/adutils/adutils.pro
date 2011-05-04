#-------------------------------------------------
#
# Project created by QtCreator 2010-12-26T13:35:38
#
#-------------------------------------------------

QT       -= core gui

TARGET = adutils
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../../libs
include (../../boost.pri)

SOURCES += adutils.cpp \
    processeddata.cpp

HEADERS += adutils.h \
    processeddata.h
