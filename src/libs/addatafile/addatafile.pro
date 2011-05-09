#-------------------------------------------------
#
# Project created by QtCreator 2011-02-12T16:13:21
#
#-------------------------------------------------

QT       -= gui

TARGET = addatafile
TEMPLATE = lib

DEFINES += ADDATAFILE_LIBRARY

include(../../boost.pri)
include(../../qtplatz_library.pri)
INCLUDEPATH += ../

SOURCES += addatafile.cpp \
    datafile.cpp \
    datafile_factory.cpp \
    copyin_visitor.cpp

HEADERS += addatafile.hpp \
        addatafile_global.h \
    datafile.hpp \
    datafile_factory.hpp \
    copyin_visitor.hpp
