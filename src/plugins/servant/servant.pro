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

SOURCES += servant.cpp \
    servantplugin.cpp \
    servantuimanager.cpp \
    servantmode.cpp \
    mainwindow.cpp

HEADERS += servant_global.h \
	servant.hpp \
    servantplugin.hpp \
    servantuimanager.hpp \
    servantmode.hpp

OTHER_FILES += \
    servant.pluginspec \
    servant.config.xml

