#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T22:32:02
#
#-------------------------------------------------

QT       += xml

TARGET = adwidgets
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatz_library_rule.pri)

SOURCES +=  peakresultwidget.cpp

HEADERS +=  peakresultwidget.hpp

include(../../boost.pri)

OTHER_FILES += \
    adwidgets.pri \
    boost.pri
