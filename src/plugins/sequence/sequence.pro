#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:59:52
#
#-------------------------------------------------

QT       += xml

TARGET = sequence
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)

include(sequence_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)
#LIBS += -ladplugin -ladportable -ladcontrols -lacewrapper -lqtwrapper
LIBS += -l$$qtLibraryTarget(adcontroller) -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adutils) -l$$qtLibraryTarget(adinterface) \
    -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adwplot) \
    -l$$qtLibraryTarget(acewrapper) -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(adplugin)

DEFINES += SEQUENCE_LIBRARY

SOURCES +=  sequenceplugin.cpp \
    sequenceeditor.cpp \
    sequenceeditorfactory.cpp \
    sequencemode.cpp \
    sequence.cpp \
    sequencemanager.cpp

HEADERS += sequence_global.h \
    sequenceplugin.h \
    sequenceeditor.h \
    sequenceeditorfactory.h \
    sequencemode.h \
    constants.h \
    sequence.h \
    sequencemanager.h

OTHER_FILES += sequence.pluginspec \
    sequence-mimetype.xml \
    sequence_dependencies.pri

RESOURCES += \
    sequence.qrc
