#-------------------------------------------------
#
# Project created by QtCreator 2012-11-06T10:46:36
#
#-------------------------------------------------

QT       -= gui core

TARGET = adchem
TEMPLATE = lib
DEFINES += ADCHEM_LIBRARY

win32: QMAKE_CXXFLAGS += -wd4100

include(../../qtplatzlibrary.pri)
include(../../boost.pri)
include(../../openbabel.pri)

QMAKE_CXXFLAGS -= -std=c++11
QMAKE_CXXFLAGS -= -stdlib=libc++
QMAKE_LFLAGS -= -stdlib=libc++

SOURCES += adchem.cpp \
    attribute.cpp \
    attributes.cpp \
    mol.cpp \
    conversion.cpp \
    chopper.cpp \
    string.cpp

HEADERS += adchem.hpp \
    attribute.hpp \
    attributes.hpp \
    mol.hpp \
    conversion.hpp \
    chopper.hpp \
    string.hpp
