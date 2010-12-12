#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:15:52
#
#-------------------------------------------------

QT       -= core gui
QT  += xml

TARGET = xmlwrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../adilibrary.pri)

SOURCES += qtxml.cpp \
    msxml.cpp \
    xtree.cpp

HEADERS += xmldom.h \
    qtxml.h \
    msxml.h \
    xtree.h

OTHER_FILES += \
    xmlwrapper.pri
