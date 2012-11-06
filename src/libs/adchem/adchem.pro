#-------------------------------------------------
#
# Project created by QtCreator 2012-11-06T10:46:36
#
#-------------------------------------------------

QT       -= core gui

TARGET = adchem
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatz_library.pri)
include(../../boost.pri)
include(../../openbabel.pri)

SOURCES += adchem.cpp \
    mol.cpp \
    conversion.cpp \
    chopper.cpp

HEADERS += adchem.hpp \
    mol.hpp \
    conversion.hpp \
    chopper.hpp
