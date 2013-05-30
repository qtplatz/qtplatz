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

include(../../qtplatzstaticlib.pri)

SOURCES +=  pugiwrapper.cpp \
#        qtxml.cpp \
        pugixml.cpp

HEADERS +=  pugiconfig.hpp \
        pugixml.hpp \
        qtxml.hpp \
        pugiwrapper.hpp

OTHER_FILES += \
    xmlparser.pri
