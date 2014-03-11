#-------------------------------------------------
#
# Project created by QtCreator 2014-02-25T14:26:54
#
#-------------------------------------------------

QT       -= gui

TARGET = u5303a
TEMPLATE = lib

include(../../agilentlibrary.pri)
include(../../boost.pri)

DEFINES += U5303A_LIBRARY

SOURCES += digitizer.cpp \
        simulator.cpp \
        waveform_generator.cpp

HEADERS += u5303a_global.hpp \
        digitizer.hpp \
        safearray.hpp \
        simulator.hpp \
        waveform_generator.hpp

LIBS += -l$$qtLibraryTarget( adlog ) \
        -l$$qtLibraryTarget( adportable )

unix {
    target.path = /usr/lib
    INSTALLS += target
}
