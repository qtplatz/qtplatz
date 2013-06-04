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

win32 {
  LIBS += -l$$qtLibraryTarget( ACE ) \
          -l$$qtLibraryTarget( TAO ) \
          -l$$qtLibraryTarget( TAO_Utils ) \
          -l$$qtLibraryTarget( TAO_PI ) \
          -l$$qtLibraryTarget( TAO_PortableServer ) \
          -l$$qtLibraryTarget( TAO_AnyTypeCode )
} else {
  LIBS += -lACE
  LIBS += -lTAO -lTAO_Utils -lTAO_PI -lTAO_PortableServer -lTAO_AnyTypeCode
  LIBS += -lboost_filesystem -lboost_system
}

DEFINES += ADPLUGIN_LIBRARY

SOURCES += adplugin.cpp \
    lifecycle.cpp \
    manager.cpp \
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
    qobserverevents_i.hpp \
    qbrokersessionevent.hpp \
    lifecycleaccessor.hpp \
    loader.hpp \
    plugin.hpp \
    orbfactory.hpp \
    orbservant.hpp \
    plugin_ptr.hpp \
    visitor.hpp
