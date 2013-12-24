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

LIBS += -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser)

!win32 {
  LIBS += -lboost_filesystem -lboost_system -lboost_regex
}

DEFINES += ADPLUGIN_LIBRARY

SOURCES += \
    lifecycle.cpp \
    manager.cpp \
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
    widget_factory.hpp \
    lifecycle.hpp \
    lifecycleaccessor.hpp \
    loader.hpp \
    plugin.hpp \
    orbfactory.hpp \
    orbservant.hpp \
    plugin_ptr.hpp \
    visitor.hpp
