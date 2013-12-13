#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T10:41:41
#
#-------------------------------------------------

QT       += widgets svg

TARGET = adwchem
TEMPLATE = lib
include(../../boost.pri)
include(../../rdkit.pri)

DEFINES += ADWCHEM_LIBRARY

SOURCES += adwchem.cpp \
        molwidget.cpp

HEADERS += adwchem.hpp\
        adwchem_global.hpp \
        molwidget.hpp

LIBS += -lFileParsers \
        -lGraphMol \
        -lSmilesParse \
        -lRDGeneral \
        -lRDGeometryLib \
        -lSubstructMatch \
        -lDepictor \
        -lDescriptors

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
