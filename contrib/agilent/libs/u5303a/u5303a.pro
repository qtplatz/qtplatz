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
IVIROOTDIR = "C:/Program Files/IVI Foundation/IVI"
VXIPNPPATH = "C:/Program Files/IVI Foundation/VISA"
#             C:\Program Files\IVI Foundation\VISA\VisaCom64

DEFINES += U5303A_LIBRARY
INCLUDEPATH += $${IVIROOTDIR}/Include \
               $${IVIROOTDIR}/Bin \
               $${VXIPNPPATH}/WinNT/include \
               $${VXIPNPPATH}/WinNT/agvisa/include \
               $${VXIPNPPATH}/VisaCom64


message("includepath=" $${INCLUDEPATH})

SOURCES += digitizer.cpp \
        simulator.cpp \
        waveform_generator.cpp \
        sampleprocessor.cpp

HEADERS += u5303a_global.hpp \
        digitizer.hpp \
        safearray.hpp \
        simulator.hpp \
        waveform_generator.hpp \
        sampleprocessor.hpp

LIBS += -l$$qtLibraryTarget( adlog ) \
        -l$$qtLibraryTarget( adfs ) \
        -l$$qtLibraryTarget( adutils ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adcontrols )

unix {
    target.path = /usr/lib
    INSTALLS += target
}
