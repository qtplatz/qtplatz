#-------------------------------------------------
#
# Project created by QtCreator 2011-01-12T11:13:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = adtxtfactory
TEMPLATE = lib

INCLUDEPATH += ../../libs
include (../../boost.pri)

DEFINES += ADTXTFACTORY_LIBRARY

SOURCES += adtxtfactory.cpp \
    datafile.cpp \
    datafile_factory.cpp \
    txtspectrum.cpp

HEADERS += adtxtfactory.h\
    adtxtfactory_global.h \
    datafile.h \
    datafile_factory.h \
    txtspectrum.h
