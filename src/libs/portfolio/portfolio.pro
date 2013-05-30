#-------------------------------------------------
#
# Project created by QtCreator 2010-12-12T10:21:31
#
#-------------------------------------------------

QT       -= core gui

TARGET = portfolio
TEMPLATE = lib

INCLUDEPATH += ../../libs
include(../../boost.pri)
include(../../qtplatzlibrary.pri)

LIBS *= -L$$IDE_LIBRARY_PATH 
LIBS += -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(adportable)

win32 {
    LIBS *= -lole32
}

DEFINES += PORTFOLIO_LIBRARY

SOURCES += portfolio.cpp \
    folder.cpp \
    folium.cpp \
    node.cpp \
    portfolioimpl.cpp

HEADERS += portfolio.hpp \
    portfolio_global.h \
    folder.hpp \
    folium.hpp \
    portfolioimpl.hpp
