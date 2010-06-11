# -------------------------------------------------
# Project created by QtCreator 2010-06-05T17:24:25
# -------------------------------------------------
QT += svg \
    xml \
    webkit

TARGET = dataanalysis
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtPlatzplugin.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)
DEFINES += DATAANALYSIS_LIBRARY
SOURCES += dataanalysisplugin.cpp \
    dataanalysiswindow.cpp \
    datafile.cpp
HEADERS += dataanalysisplugin.h \
    dataanalysiswindow.h \
    datafile.h
OTHER_FILES += dataanalysis.pluginspec
