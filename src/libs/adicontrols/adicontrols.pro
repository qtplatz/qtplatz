#-------------------------------------------------
#
# Project created by QtCreator 2010-06-26T16:45:23
#
#-------------------------------------------------

QT       -= gui

TARGET = adicontrols
TEMPLATE = lib

include(../../adilibrary.pri)

DEFINES += ADICONTROLS_LIBRARY

SOURCES += massspectrum.cpp

HEADERS += massspectrum.h\
        adicontrols_global.h \
    import_sacontrols.h

OTHER_FILES += \
    adicontrols.pri
