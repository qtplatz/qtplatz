#-------------------------------------------------
#
# Project created by QtCreator 2010-08-05T16:33:48
#
#-------------------------------------------------

QT       += gui

TARGET = adplugin
TEMPLATE = lib
include(../../adilibrary.pri)
include(../../boost.pri)
INCLUDEPATH += $(ACE_ROOT) $(TAO_ROOT) $(TAO_ROOT)/orbsvcs
LIBS += -L$(ACE_ROOT)/lib -L../../../lib/qtPlatz

DEFINES += ADPLUGIN_LIBRARY

SOURCES += adplugin.cpp \
    lifecycle.cpp \
    qreceiver_i.cpp \
    configloader.cpp \
    orbmanager.cpp


HEADERS += adplugin.h\
        adplugin_global.h \
    imonitor.h \
    icontrolmethodeditor.h \
    ifactory.h \
    lifecycle.h \
    qreceiver_i.h \
    configloader.h \
    orbmanager.h \
    orbLoader.h
