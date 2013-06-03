#-------------------------------------------------
#
# Project created by QtCreator 2010-08-05T16:33:48
#
#-------------------------------------------------

QT       += gui
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

TARGET = adplugin
TEMPLATE = lib
include(../../qtplatzlibrary.pri)
include(../../ace_tao.pri)
include(../../boost.pri)
INCLUDEPATH += $$OUT_PWD/..

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
    lifecycleaccessor.cpp \
    loader.cpp \
    plugin.cpp \
    orbfactory.cpp \
    orbservant.cpp \
    plugin_ptr.cpp \
    visitor.cpp \
    widget_factory.cpp

HEADERS += adplugin.hpp \
    adplugin_global.h \
    imonitor.hpp \
    icontrolmethodeditor.hpp \
    widget_factory.hpp \
    lifecycle.hpp \
    qreceiver_i.hpp \
    orbmanager.hpp \
    orbLoader.hpp \
    qobserverevents_i.hpp \
    qbrokersessionevent.hpp \
    lifecycleaccessor.hpp \
    loader.hpp \
    plugin.hpp \
    orbfactory.hpp \
    orbservant.hpp \
    plugin_ptr.hpp \
    visitor.hpp
