#-------------------------------------------------
#
# Project created by QtCreator 2012-04-19T16:35:38
#
#-------------------------------------------------

QT       -= core gui

#TARGET = mzxml
TEMPLATE = lib

include(../contrib.pri)

TARGET = $$qtLibraryTarget($$TARGET)
TEMPLATE = lib

INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src
DESTDIR = $$QTPLATZ_PLUGIN_PATH/$$PROVIDER

DEFINES += MZXML_LIBRARY

SOURCES += mzxml.cpp \
    datafile_factory.cpp \
    datafile.cpp

HEADERS += mzxml.hpp\
        mzxml_global.hpp \
    datafile_factory.hpp \
    datafile.hpp


