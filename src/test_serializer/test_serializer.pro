#-------------------------------------------------
#
# Project created by QtCreator 2010-07-04T09:16:29
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = test_serializer
CONFIG   += console
CONFIG   -= app_bundle
include(../adilibrary.pri)
TEMPLATE = app
include(../boost.pri)
LIBS += -l$$qtLibraryTarget(adcontrolsd)
LIBS += -L$$IDE_LIBRARY_PATH
INCLUDEPATH += ../libs

SOURCES += main.cpp
