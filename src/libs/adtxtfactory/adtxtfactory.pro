#-------------------------------------------------
#
# Project created by QtCreator 2011-01-12T11:13:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = adtxtfactory
TEMPLATE = lib
include(../../qtplatz_servant.pri)

INCLUDEPATH += ../../libs
include (../../boost.pri)
LIBS += -lboost_system

DEFINES += ADTXTFACTORY_LIBRARY

SOURCES += adtxtfactory.cpp \
    datafile.cpp \
    datafile_factory.cpp \
    txtspectrum.cpp

HEADERS += adtxtfactory.hpp \
    adtxtfactory_global.h \
    datafile.hpp \
    datafile_factory.hpp \
    txtspectrum.hpp
