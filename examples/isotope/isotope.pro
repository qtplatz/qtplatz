#-------------------------------------------------
#
# Project created by QtCreator 2014-02-12T08:48:05
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = isotope
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../src/boost.pri)

SOURCES += main.cpp \
    element.cpp \
    isotope.cpp \
    tableofelement.cpp

HEADERS += \
    element.hpp \
    isotope.hpp \
    tableofelement.hpp
