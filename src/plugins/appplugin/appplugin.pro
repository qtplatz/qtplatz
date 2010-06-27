#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T10:22:54
#
#-------------------------------------------------

QT       += xml

TARGET = appplugin
TEMPLATE = lib
PROVIDER = ScienceLiaison

include(../../adiplugin.pri)
include(appplugin_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += APPPLUGIN_LIBRARY

SOURCES += appplugin.cpp \
    outputwindow.cpp

HEADERS += appplugin.h\
        appplugin_global.h \
    outputwindow.h

OTHER_FILES += \
    appplugin.pri \
    appplugin_dependencies.pri \
    appplugin.pluginspec
