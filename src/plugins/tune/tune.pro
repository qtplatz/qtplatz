#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:03:35
#
#-------------------------------------------------

QT       += xml

TARGET = tune
TEMPLATE = lib
PROVIDER = ScienceLiaison

include(../../qtPlatzplugin.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += TUNE_LIBRARY

SOURCES += tune.cpp

HEADERS += tune.h\
        tune_global.h

OTHER_FILES += sequence.pluginspec
