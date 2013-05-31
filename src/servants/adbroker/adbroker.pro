#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T11:29:18
#
#-------------------------------------------------

QT       -= gui

TARGET = adbroker
TEMPLATE = lib

include(../../adplugin.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

INCLUDEPATH *= $$OUT_PWD/../../libs

!win32: LIBS += -lboost_date_time

LIBS += -l$$qtLibraryTarget(adinterface) \
    -l$$qtLibraryTarget(adportable) \
    -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(portfolio) \
    -l$$qtLibraryTarget(adplugin)

LIBS += \
    -l$$qtLibraryTarget(ACE) \
    -l$$qtLibraryTarget(TAO)

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
    task.cpp \
    objectdiscovery.cpp

HEADERS += adbroker.hpp \
    adbroker_global.h \
    brokermanager.hpp \
    brokerconfig.hpp \
    brokermanager.hpp \
    brokersession.hpp \
    brokertoken.hpp \
    chemicalformula_i.hpp \
    logger_i.hpp \
    manager_i.hpp \
    message.hpp \
    session_i.hpp \
    task.hpp \
    objectdiscovery.hpp

OTHER_FILES += \
    adbroker.pri \
    adbroker_dependencies.pri


