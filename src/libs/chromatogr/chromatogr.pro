#-------------------------------------------------
#
# Project created by QtCreator 2012-05-02T10:35:56
#
#-------------------------------------------------

QT       -= gui

TARGET = chromatogr
TEMPLATE = lib

include(../../qtplatzlibrary.pri)
include(../../boost.pri)

LIBS += -l$$qtLibraryTarget(adportable)
LIBS += -l$$qtLibraryTarget(adcontrols)

DEFINES += CHROMATOGR_LIBRARY

SOURCES += chromatogr.cpp \
    chromatography.cpp \
    integrator.cpp

HEADERS += chromatogr.hpp \
           chromatogr_global.hpp \
           chromatography.hpp \
           differential.hpp \
           averager.hpp \
           integrator.hpp
