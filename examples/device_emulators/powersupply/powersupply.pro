#-------------------------------------------------
#
# Project created by QtCreator 2011-12-03T11:47:40
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = powersupply
CONFIG   += console
CONFIG   -= app_bundle
LIBS *= -lboost_system -lboost_date_time
TEMPLATE = app


SOURCES += main.cpp \
    bcast_server.cpp \
    dgram_server.cpp \
    mcast_receiver.cpp \
    mcast_sender.cpp

HEADERS += \
    bcast_server.hpp \
    dgram_server.hpp \
    mcast_receiver.hpp \
    mcast_sender.hpp








