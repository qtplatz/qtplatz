#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:46:27
#
#-------------------------------------------------

QT       += xml

TARGET = acquire
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtPlatzplugin.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ACQUIRE_LIBRARY

SOURCES += acquire.cpp

HEADERS += acquire.h\
        acquire_global.h

OTHER_FILES += acquire.pluginspec
