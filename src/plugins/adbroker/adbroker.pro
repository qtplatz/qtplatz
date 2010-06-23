#-------------------------------------------------
#
# Project created by QtCreator 2010-06-23T10:06:16
#
#-------------------------------------------------

QT       += xml

QT       -= gui

TARGET = adbroker
TEMPLATE = lib
PROVIDER = ScienceLiaison

include(../../qtPlatzplugin.pri)
include(adbroker_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ADBROKER_LIBRARY

SOURCES += adbrokerplugin.cpp

HEADERS += adbrokerplugin.h\
        adbroker_global.h

OTHER_FILES += \
    adbroker.pluginspec \
    adbroker_dependencies.pri
