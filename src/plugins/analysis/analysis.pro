#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

QT       += xml

TARGET = analysis
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtPlatzplugin.pri)
include(analysis_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ANALYSIS_LIBRARY

SOURCES += analysis.cpp \
    analysisplugin.cpp

HEADERS += analysis.h\
        analysis_global.h \
    analysisplugin.h

OTHER_FILES += analysis.pluginspec \
    analysis_dependencies.pri
