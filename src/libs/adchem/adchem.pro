#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T12:37:15
#
#-------------------------------------------------

QT       -= core gui

TARGET = adchem
TEMPLATE = lib
INCLUDEPATH += ../../libs
include(../../qtplatzlibrary.pri)

include(../../boost.pri)
include(../../rdkit.pri)

DEFINES += ADCHEM_LIBRARY

SOURCES += adchem.cpp \
    sdfile.cpp \
    molfile.cpp \
    drawing.cpp \
    molecule.cpp

HEADERS += adchem.hpp\
        adchem_global.hpp \
    sdfile.hpp \
    molfile.hpp \
    drawing.hpp \
    molecule.hpp

LIBS += -lFileParsers \
        -lGraphMol \
        -lSmilesParse \
        -lRDGeneral \
        -lRDGeometryLib \
        -lSubstructMatch \
        -lDepictor \
        -lDescriptors
!win32 {
  LIBS += -lboost_system
}

LIBS += -l$$qtLibraryTarget( adportable )


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
