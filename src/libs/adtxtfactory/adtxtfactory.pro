#-------------------------------------------------
#
# Project created by QtCreator 2011-01-12T11:13:43
#
#-------------------------------------------------

QT       -= gui

TARGET = adtxtfactory
TEMPLATE = lib
include(../../adplugin.pri)
include (../../boost.pri)

INCLUDEPATH += ../../libs

LIBS += -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(xmlparser) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(adplugin)

!win32 {
  LIBS += -lboost_system -lboost_filesystem
}

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
