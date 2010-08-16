#-------------------------------------------------
#
# Project created by QtCreator 2010-07-30T12:33:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = adcontroller
TEMPLATE = lib

DEFINES += ADCONTROLLER_LIBRARY

SOURCES += adcontroller.cpp \
        ibroker.cpp \
        ibrokermanager.cpp \
        manager_i.cpp \
        mcast_handler.cpp \
        message.cpp \
        receiver_i.cpp \
        session_i.cpp \
        signal_handler.cpp \
    iproxy.cpp

HEADERS += adcontroller.h\
        adcontroller_global.h \
        constants.h \
        ibroker.h \
        ibrokermanager.h \
        manager_i.h \
        marshal.hpp \
        mcast_handler.h \
        message.h \
        receiver_i.h \
        session_i.h \
        signal_handler.h \
    iproxy.h
