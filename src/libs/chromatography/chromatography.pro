#-------------------------------------------------
#
# Project created by QtCreator 2012-05-02T10:23:45
#
#-------------------------------------------------

QT       -= core gui

TARGET = chromatography
TEMPLATE = lib

include(../../qtplatz_library.pri)
include(../../boost.pri)
#include(../../ace_tao.pri)
#include(../acewrapper/acewrapper_dependencies.pri)
#LIBS *= -l$$qtLibraryTarget(acewrapper)
LIBS *= -l$$qtLibraryTarget(adportable)
LIBS *= -l$$qtLibraryTarget(adcontrols)

DEFINES += CHROMATOGRAPHY_LIBRARY

SOURCES += chromatography.cpp

HEADERS += chromatography.hpp\
        chromatography_global.hpp

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEB27803D
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = chromatography.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
