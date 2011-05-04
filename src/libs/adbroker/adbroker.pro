#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T11:29:18
#
#-------------------------------------------------

QT       -= gui

TARGET = adbroker
TEMPLATE = lib

include(../../qtplatz_libs.pri)

DEFINES += ADBROKER_LIBRARY

SOURCES += brokermanager.cpp \
    brokersession.cpp \
    brokertoken.cpp \
    brokerconfig.cpp

HEADERS += brokermanager.h\
        adbroker_global.h \
    brokersession.h \
    brokertoken.h \
    brokerconfig.h

OTHER_FILES += \
    adbroker.pri \
    adbroker_dependencies.pri
