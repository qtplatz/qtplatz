#-------------------------------------------------
#
# Project created by QtCreator 2010-08-11T12:03:35
#
#-------------------------------------------------

TARGET = servant
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../adiplugin.pri)
include(servant_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += SERVANT_LIBRARY

SOURCES += servant.cpp \
    servantplugin.cpp \
    servantuimanager.cpp \
    servantmode.cpp \
    mainwindow.cpp

HEADERS += servant.h\
        servant_global.h \
    servantplugin.h \
    servantuimanager.h \
    servantmode.h \
    mainwindow.h

OTHER_FILES += \
    servant.pluginspec \
    servant.config.xml

FORMS += \
    mainwindow.ui
