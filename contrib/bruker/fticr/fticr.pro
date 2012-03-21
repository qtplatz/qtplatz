#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T12:50:00
#
#-------------------------------------------------

QT       -= core gui

include(../../contrib.pri)

TARGET = fticr
TEMPLATE = lib

INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src

DEFINES += FTICR_LIBRARY

SOURCES += fticr.cpp \
    datafile_factory.cpp \
    datafile.cpp \
    jcampdxparser.cpp

HEADERS += fticr.hpp\
        fticr_global.hpp \
    datafile_factory.hpp \
    datafile.hpp \
    jcampdxparser.hpp

