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
        iproxy.cpp \
        manager_i.cpp \
        mcast_handler.cpp \
        message.cpp \
        observer_i.cpp \
        oproxy.cpp \
        receiver_i.cpp \
        session_i.cpp \
        signal_handler.cpp \
    cache.cpp

HEADERS += adcontroller.h\
        adcontroller_global.h \
        constants.h \
        ibroker.h \
        ibrokermanager.h \
        iproxy.h \
        manager_i.h \
        marshal.hpp \
        mcast_handler.h \
        message.h \
        observer_i.h \
        oproxy.h \
        receiver_i.h \
        session_i.h \
        signal_handler.h \
    cache.h
