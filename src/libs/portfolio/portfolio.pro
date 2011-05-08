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
include(../../qtplatz_library_rule.pri)
LIBS *= -L$$IDE_LIBRARY_PATH -l$$qtLibraryTarget(xmlparser) -lole32

DEFINES += PORTFOLIO_LIBRARY

SOURCES += portfolio.cpp \
    folder.cpp \
    folium.cpp \
    node.cpp \
    portfolioimpl.cpp

HEADERS += portfolio.h\
    portfolio_global.h \
    folder.h \
    folium.h \
    portfolioimpl.h
