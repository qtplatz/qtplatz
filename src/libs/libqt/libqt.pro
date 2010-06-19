#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T22:32:02
#
#-------------------------------------------------
include(../../qtPlatz.pri)

QT       += xml

TARGET = libqt
TEMPLATE = lib
CONFIG += staticlib

DESTDIR = $$IDE_LIBRARY_PATH

include(rpath.pri)

SOURCES += libqt.cpp \
    tracewidget.cpp

HEADERS += libqt.h \
    tracewidget.h

INCLUDEPATH += /usr/local/include c:/Boost/include/boost-1_43

target.path = /$$IDE_LIBRARY_BASENAME/qtPlatz
INSTALLS += target
