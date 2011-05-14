#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:03:35
#
#-------------------------------------------------

QT       += xml

TARGET = tune
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)

LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += TUNE_LIBRARY

SOURCES += tune.cpp \
    tuneplugin.cpp

HEADERS += tune.h\
        tune_global.h \
    tuneplugin.h

OTHER_FILES += tune.pluginspec
