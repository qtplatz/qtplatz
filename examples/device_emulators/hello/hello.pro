#-------------------------------------------------
#
# Project created by QtCreator 2011-12-03T16:14:10
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = hello
CONFIG   += console
CONFIG   -= app_bundle
LIBS *= -lboost_system -lboost_date_time

TEMPLATE = app


SOURCES += main.cpp \
        mcast_receiver.cpp \
    dgram_server.cpp

HEADERS += \
    mcast_receiver.hpp \
    dgram_server.hpp



