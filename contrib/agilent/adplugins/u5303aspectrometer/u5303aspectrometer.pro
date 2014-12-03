#-------------------------------------------------
#
# Project created by QtCreator 2014-12-03T11:26:00
#
#-------------------------------------------------

QT       -= gui

TARGET = u5303aspectrometer
TEMPLATE = lib

DEFINES += U5303ASPECTROMETER_LIBRARY

SOURCES += u5303aspectrometer.cpp

HEADERS += u5303aspectrometer.hpp\
        u5303aspectrometer_global.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
