#-------------------------------------------------
#
# Project created by QtCreator 2010-08-11T12:03:35
#
#-------------------------------------------------

TARGET = servant
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(servant_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += SERVANT_LIBRARY

SOURCES += logger.cpp \
        orbservantmanager.cpp \
        outputwindow.cpp \
        servant.cpp \
    servantmode.cpp \
    servantplugin.cpp \
    servantpluginimpl.cpp \
    servantuimanager.cpp \
    mainwindow.cpp

HEADERS += servant_global.h \
        logger.hpp \
    orbservantmanager.hpp \
    outputwindow.hpp \
    servant.hpp \
    servantmode.hpp \
    servantplugin.hpp \
    servantpluginimpl.hpp \
    servantuimanager.hpp \
    mainwindow.hpp

OTHER_FILES += \
    servant.pluginspec \
    servant.config.xml

