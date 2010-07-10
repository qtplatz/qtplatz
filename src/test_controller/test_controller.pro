#-------------------------------------------------
#
# Project created by QtCreator 2010-07-07T14:07:09
#
#-------------------------------------------------

QT       += core gui

TARGET = test_controller
TEMPLATE = app
include(../boost.pri)
include(../adilibrary.pri)

INCLUDEPATH += $$(ACE_ROOT) ../libs

LIBS *= -L$$IDE_LIBRARY_PATH -L$$(ACE_ROOT)/lib
Debug {
    LIBS += -lacewrapperd -lACEd
    CONFIG += debug
}
Release {
    LIBS += -lacewrapper -lACE
}

SOURCES += main.cpp\
        maincontrollerwindow.cpp

HEADERS  += maincontrollerwindow.h

FORMS    += maincontrollerwindow.ui


#CONFIG   += console
#CONFIG   -= app_bundle
#include(../adilibrary.pri)
#LIBS += -l$$qtLibraryTarget(adcontrolsd)

