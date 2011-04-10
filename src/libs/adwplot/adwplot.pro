#-------------------------------------------------
#
# Project created by QtCreator 2011-04-10T07:13:16
#
#-------------------------------------------------

QT       -= gui

TARGET = adwplot
TEMPLATE = lib
CONFIG += staticlib

SOURCES += adwplot.cpp \
    dataplot.cpp \
    trace.cpp \
    zoomer.cpp \
    axis.cpp \
    traces.cpp \
    title.cpp \
    titles.cpp

HEADERS += adwplot.h \
    dataplot.h \
    trace.h \
    zoomer.h \
    axis.h \
    traces.h \
    title.h \
    titles.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
