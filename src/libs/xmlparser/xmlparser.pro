#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:15:52
#
#-------------------------------------------------

QT       -= core gui
QT  += xml

TARGET = xmlparser
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatz_library_rule.pri)

SOURCES +=  pugiwrapper.cpp \
#        qtxml.cpp \
        pugixml.cpp

HEADERS +=  pugiconfig.hpp \
        pugixml.hpp \
        qtxml.h \
        pugiwrapper.h

OTHER_FILES += \
    xmlparser.pri
