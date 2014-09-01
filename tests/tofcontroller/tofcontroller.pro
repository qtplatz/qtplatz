#-------------------------------------------------
#
# Project created by QtCreator 2010-08-12T19:51:50
#
#-------------------------------------------------

QT       -= core gui

TARGET = tofcontroller
TEMPLATE = lib

DEFINES += TOFCONTROLLER_LIBRARY

SOURCES += tofcontroller.cpp \
           deviceproxy.cpp \
           tofcontroller.cpp \
           tofobserver_i.cpp \
           tofsession_i.cpp \
           traceobserver_i.cpp

HEADERS += tofcontroller.h\
        tofcontroller_global.h \
        analyzerdevicedata.h \
        constants.h \
        deviceproxy.h \
        marshal.hpp \
        tofobserver_i.h \
        toftask.h \
        traceobserver_i.h
