#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T22:32:02
#
#-------------------------------------------------

QT       += xml

TARGET = adwidgets
TEMPLATE = lib
CONFIG += staticlib qaxcontainer

#DESTDIR = $$IDE_LIBRARY_PATH
include(../../qtPlatzlibrary.pri)

SOURCES += libqt.cpp \
    tracewidget.cpp

HEADERS += libqt.h \
    tracewidget.h

include(../../boost.pri)

OTHER_FILES += \
    libadwidgets.pri \
    boost.pri
