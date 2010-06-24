#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

QT       += xml

TARGET = analysis
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../adiplugin.pri)
include(analysis_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ANALYSIS_LIBRARY

SOURCES += analysis.cpp \
    analysisplugin.cpp \
    analysismode.cpp \
    analysismanager.cpp \
    msprocessingwnd.cpp \
    elementalcompwnd.cpp \
    mscalibrationwnd.cpp \
    chromatogramwnd.cpp

HEADERS += analysis.h\
        analysis_global.h \
    analysisplugin.h \
    analysismode.h \
    analysismanager.h \
    msprocessingwnd.h \
    elementalcompwnd.h \
    mscalibrationwnd.h \
    chromatogramwnd.h

OTHER_FILES += analysis.pluginspec \
    analysis_dependencies.pri
