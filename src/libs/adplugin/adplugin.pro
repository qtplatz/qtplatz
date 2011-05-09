#-------------------------------------------------
#
# Project created by QtCreator 2010-08-05T16:33:48
#
#-------------------------------------------------

QT       += gui

TARGET = adplugin
TEMPLATE = lib
include(../../qtplatz_library.pri)
include(../../boost.pri)
INCLUDEPATH += $(ACE_ROOT) $(TAO_ROOT) $(TAO_ROOT)/orbsvcs
LIBS += -L$(ACE_ROOT)/lib

DEFINES += ADPLUGIN_LIBRARY

SOURCES += adplugin.cpp \
    lifecycle.cpp \
        manager.cpp \
        orbmanager.cpp \
        qbrokersessionevent.cpp \
        qobserverevents_i.cpp \
        qreceiver_i.cpp

HEADERS += adplugin.h\
        adplugin_global.h \
    imonitor.hpp \
    icontrolmethodeditor.hpp \
    ifactory.hpp \
    lifecycle.hpp \
    qreceiver_i.hpp \
    configloader.hpp \
    orbmanager.hpp \
    orbLoader.hpp \
    qobserverevents_i.hpp \
    qbrokersessionevent.hpp
