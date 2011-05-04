#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T22:32:02
#
#-------------------------------------------------

QT       += xml

TARGET = adwidgets
TEMPLATE = lib
CONFIG += staticlib

#DESTDIR = $$IDE_LIBRARY_PATH
include(../../adilibrary.pri)

SOURCES +=  peakresultwidget.cpp

HEADERS +=  peakresultwidget.h

include(../../boost.pri)

OTHER_FILES += \
    adwidgets.pri \
    boost.pri
