#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T19:01:24
#
#-------------------------------------------------

QT       += xml

TARGET = batchproc
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtPlatzplugin.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += BATCHPROC_LIBRARY

SOURCES += batchproc.cpp \
    batchprocplugin.cpp

HEADERS += batchproc.h\
        batchproc_global.h \
    batchprocplugin.h

OTHER_FILES += batchproc.pluginspec
