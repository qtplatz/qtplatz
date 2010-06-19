#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:46:27
#
#-------------------------------------------------

QT       += xml libqt

TARGET = acquire
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtPlatzplugin.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia

include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ACQUIRE_LIBRARY

SOURCES +=  acquireplugin.cpp \
    acquiremode.cpp

HEADERS +=  acquire_global.h \
    acquireplugin.h \
    acquiremode.h

OTHER_FILES += acquire.pluginspec
