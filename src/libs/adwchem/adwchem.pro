#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T10:41:41
#
#-------------------------------------------------

QT       += widgets svg

TARGET = adwchem
TEMPLATE = lib
include(../../qtplatzlibrary.pri)
include(../../boost.pri)
include(../../rdkit.pri)

DEFINES += ADWCHEM_LIBRARY

SOURCES += adwchem.cpp \
        molwidget.cpp

HEADERS += adwchem.hpp\
        adwchem_global.hpp \
        molwidget.hpp

LIBS += -l$$qtLibraryTarget( adchem ) \
        -l$$qtLibraryTarget( adportable )

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
