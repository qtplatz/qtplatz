#-------------------------------------------------
#
# Project created by QtCreator 2014-08-20T08:12:13
#
#-------------------------------------------------
include(../../../qtplatz.pri)
include(../../boost.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = publisher
TEMPLATE = app

SOURCES += main.cpp\
           mainwindow.cpp

HEADERS  += mainwindow.hpp

FORMS    += mainwindow.ui

INCLUDEPATH += $${QTPLATZ_SOURCE_TREE}/src/libs
LIBS += -l$$qtLibraryTarget(xmlparser) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adpublisher) 

win32 {
  DESTDIR = $$IDE_BIN_PATH
}

RESOURCES += \
    publisher.qrc



