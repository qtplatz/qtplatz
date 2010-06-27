#-------------------------------------------------
#
# Project created by QtCreator 2010-06-26T16:45:23
#
#-------------------------------------------------

QT       -= gui

TARGET = adcontrols
TEMPLATE = lib

include(../../adilibrary.pri)

DEFINES += ADCONTROLS_LIBRARY

SOURCES += massspectrum.cpp

HEADERS += massspectrum.h\
        adcontrols_global.h \
    import_sacontrols.h

OTHER_FILES += \
    adcontrols.pri
