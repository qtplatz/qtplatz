#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T11:48:50
#
#-------------------------------------------------

QT       -= core gui

TARGET = instmanager
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../boost.pri)
include(../adilibrary.pri)

INCLUDEPATH += $$(ACE_ROOT) $$(TAO_ROOT) ../libs
LIBS *= -L$$IDE_LIBRARY_PATH -L$$(ACE_ROOT)/lib

SOURCES += main.cpp \
    signal_handler.cpp \
    session_i.cpp \
    servermanager.cpp

HEADERS += \
    signal_handler.h \
    session_i.h \
    servermanager.h \
    orbserver.h \
    instbroker.h
