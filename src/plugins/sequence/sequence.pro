#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:59:52
#
#-------------------------------------------------

QT       += xml

TARGET = sequence
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtPlatzplugin.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += SEQUENCE_LIBRARY

SOURCES += sequence.cpp \
    sequenceplugin.cpp

HEADERS += sequence.h\
        sequence_global.h \
    sequenceplugin.h

OTHER_FILES += sequence.pluginspec
