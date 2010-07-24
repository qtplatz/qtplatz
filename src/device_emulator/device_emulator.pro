#-------------------------------------------------
#
# Project created by QtCreator 2010-07-07T14:08:12
#
#-------------------------------------------------

QT       += core gui

TARGET = device_emulator
TEMPLATE = app
include(../boost.pri)
include(../adilibrary.pri)

INCLUDEPATH += $$(ACE_ROOT) $$(TAO_ROOT) ../libs
LIBS *= -L$$IDE_LIBRARY_PATH -L$$(ACE_ROOT)/lib
Debug {
    LIBS += -ladportabled -lacewrapperd -lACEd
    CONFIG += debug
}
Release {
    LIBS += -ladportable -lacewrapper -lACE
}

SOURCES += main.cpp\
        maindevicewindow.cpp \
    eventreceiver.cpp \
    roleaverager.cpp \
    roleesi.cpp \
    roleanalyzer.cpp \
    devicefacade.cpp \
    device_state.cpp

HEADERS  += maindevicewindow.h \
    eventreceiver.h \
    roleaverager.h \
    roleesi.h \
    roleanalyzer.h \
    devicefacade.h \
    device_state.h

FORMS    += maindevicewindow.ui
