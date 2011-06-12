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
INCLUDEPATH += $(ACE_ROOT)/include $(TAO_ROOT)/include $$OUT_PWD/..

LIBS += -L$(ACE_ROOT)/lib
LIBS += -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adinterface) \
    -l$$qtLibraryTarget(adportable) \
    -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser)

!win32 {
  LIBS += -lACE
  LIBS += -lTAO -lTAO_Utils -lTAO_PI -lTAO_PortableServer -lTAO_AnyTypeCode
  LIBS += -lboost_filesystem -lboost_system
}

DEFINES += ADPLUGIN_LIBRARY

SOURCES += adplugin.cpp \
    lifecycle.cpp \
        manager.cpp \
        orbmanager.cpp \
        qbrokersessionevent.cpp \
        qobserverevents_i.cpp \
        qreceiver_i.cpp \
    lifecycleaccessor.cpp

HEADERS += adplugin.hpp \
        adplugin_global.h \
    imonitor.hpp \
    icontrolmethodeditor.hpp \
    ifactory.hpp \
    lifecycle.hpp \
    qreceiver_i.hpp \
    orbmanager.hpp \
    orbLoader.hpp \
    qobserverevents_i.hpp \
    qbrokersessionevent.hpp \
    lifecycleaccessor.hpp
