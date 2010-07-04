#-------------------------------------------------
#
# Project created by QtCreator 2010-06-26T16:45:23
#
#-------------------------------------------------

QT       -= gui

TARGET = adcontrols
TEMPLATE = lib
INCLUDEPATH += $$(SATOOLS_ROOT)/bin

include(../../adilibrary.pri)
include(../../boost.pri)
include(../acewrapper/acewrapper.pri)

DEFINES += ADCONTROLS_LIBRARY

SOURCES += massspectrum.cpp \
    description.cpp \
    descriptions.cpp \
    tableofelements.cpp

HEADERS += massspectrum.h\
        adcontrols_global.h \
    import_sacontrols.h \
    adcontrols.h \
    description.h \
    descriptions.h \
    tableofelements.h

OTHER_FILES += \
    adcontrols.pri
