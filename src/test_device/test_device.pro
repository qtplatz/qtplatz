#-------------------------------------------------
#
# Project created by QtCreator 2010-07-07T14:08:12
#
#-------------------------------------------------

QT       += core gui

TARGET = test_device
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
        maindevicewindow.cpp \
    deviceevent.cpp

HEADERS  += maindevicewindow.h \
    deviceevent.h

FORMS    += maindevicewindow.ui
