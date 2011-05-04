#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:46:27
#
#-------------------------------------------------

QT       += xml

TARGET = acquire
TEMPLATE = lib
PROVIDER = ScienceLiaison

include(../../qtplatz_lib_dynamic.pri)
include(acquire_dependencies.pri)

include(../../boost.pri)

LIBS += -L$$IDE_PLUGIN_PATH/Nokia -L$$IDE_LIBRARY_PATH

include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ACQUIRE_LIBRARY

SOURCES +=  acquireplugin.cpp \
    acquiremode.cpp \
    acquireuimanager.cpp \
    acquireactions.cpp \
    session.cpp \
    acquire.cpp \
    orbmanager.cpp

HEADERS +=  acquire_global.h \
    acquireplugin.h \
    acquiremode.h \
    acquireuimanager.h \
    constants.h \
    acquireactions.h \
    session.h \
    acquire.h \
    orbmanager.h

OTHER_FILES += acquire.pluginspec \
    acquire_dependencies.pri

RESOURCES += \
    acquire.qrc
