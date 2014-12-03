#-------------------------------------------------
#
# Project created by QtCreator 2014-12-03T11:29:15
#
#-------------------------------------------------

QT       -= gui

TARGET = u5303aspectrometer
TEMPLATE = lib

DEFINES += U5303ASPECTROMETER_LIBRARY

include(../../agilentlibrary.pri)
include(../../boost.pri)

SOURCES += u5303aspectrometer.cpp \
    massspectrometer.cpp \
    datainterpreter.cpp

HEADERS += u5303aspectrometer.hpp\
        u5303aspectrometer_global.hpp \
    massspectrometer.hpp \
    datainterpreter.hpp

LIBS += -l$$qtLibraryTarget( adinterface ) \
        -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adcontrols ) \
        -l$$qtLibraryTarget( acewrapper )

unix {
    target.path = /usr/lib
    INSTALLS += target
}
