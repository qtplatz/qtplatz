#-------------------------------------------------
#
# Project created by QtCreator 2010-08-06T08:55:01
#
#-------------------------------------------------

TARGET = adtofms
TEMPLATE = lib
INCLUDEPATH += /$$IDE_LIBRARY_BASENAME/arc/libs
LIBS += -l$$qtLibraryTarget(adplugin)

PROVIDER = MS-Cheminformatics
include(../../adplugin.pri)
DEFINES += ADTOFMS_LIBRARY

SOURCES += adtofms.cpp \
    monitor_ui.cpp \
    factory_impl.cpp \
    form.cpp \
    debug_ui.cpp

HEADERS += adtofms.h\
        adtofms_global.h \
    monitor_ui.h \
    factory_impl.h \
    form.h \
    debug_ui.h

FORMS += \
    form.ui \
    debug_ui.ui
