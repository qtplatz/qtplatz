#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:15:52
#
#-------------------------------------------------

QT       -= core gui

TARGET = xmlwrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../adilibrary.pri)

SOURCES += xmldom.cpp

HEADERS += xmldom.h

OTHER_FILES += \
    xmlwrapper.pri
