#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

TARGET = qtwidgets
TEMPLATE = lib
# INCLUDEPATH += ../../libs ../../plugins
include(../../adilibrary.pri)
include(../../boost.pri)
PROVIDER = ScienceLiaison
include(../../adplugin.pri)

DEFINES += QTWIDGETS_LIBRARY

SOURCES += qtwidgets.cpp \
    logwidget.cpp \
    factory.cpp

HEADERS += qtwidgets.h\
        qtwidgets_global.h \
    logwidget.h \
    factory.h

FORMS += \
    logwidget.ui
