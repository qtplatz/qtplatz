#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T11:29:18
#
#-------------------------------------------------

QT       -= gui

TARGET = adbroker
TEMPLATE = lib

include(../../qtplatz_libs.pri)
include(../acewrapper/acewrapper_dependencies.pri)
include(../../boost.pri)

DEFINES += ADBROKER_LIBRARY

SOURCES += adbroker.cpp \
    brokerconfig.cpp \
    brokermanager.cpp \
    brokersession.cpp \
    brokertoken.cpp \
    chemicalformula_i.cpp \
    logger_i.cpp \
    manager_i.cpp \
    message.cpp \
    session_i.cpp \
    task.cpp

HEADERS += adbroker.h \
    adbroker_global.h \
    brokermanager.h \
    brokerconfig.h \
    brokermanager.h \
    brokersession.h \
    brokertoken.h \
    chemicalformula_i.h \
    logger_i.h \
    manager_i.h \
    message.h \
    session_i.h \
    task.h

OTHER_FILES += \
    adbroker.pri \
    adbroker_dependencies.pri
