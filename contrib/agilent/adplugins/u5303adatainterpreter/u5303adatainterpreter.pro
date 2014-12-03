#-------------------------------------------------
#
# Project created by QtCreator 2014-12-03T11:12:18
#
#-------------------------------------------------

QT       -= gui

TARGET = u5303adatainterpreter
TEMPLATE = lib

DEFINES += U5303ADATAINTERPRETER_LIBRARY
include(../../../src/adplutin.pri)
include(../../../src/boost.pri)

SOURCES += u5303adatainterpreter.cpp

HEADERS += u5303adatainterpreter.hpp\
        u5303adatainterpreter_global.hpp

LIBS += -l$$qtLibraryTarget( adinterface ) \
        -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adcontrols ) \
        -l$$qtLibraryTarget( acewrapper )

unix {
    target.path = /usr/lib
    INSTALLS += target
}
