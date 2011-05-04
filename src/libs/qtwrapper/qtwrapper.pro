#-------------------------------------------------
#
# Project created by QtCreator 2010-07-10T13:47:50
#
#-------------------------------------------------

QT       -= gui

TARGET = qtwrapper
TEMPLATE = lib
CONFIG += staticlib
include(../../qtplatz_lib_static.pri)

SOURCES += qtwrapper.cpp \
    qstring.cpp \
    xmldom.cpp

HEADERS += qtwrapper.h \
    qstring.h \
    xmldom.h
