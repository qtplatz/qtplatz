#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T11:29:18
#
#-------------------------------------------------

QT       -= gui

TARGET = adbroker
TEMPLATE = lib

include(../../qtplatz_library.pri)
include(../../boost.pri)
include(../acewrapper/acewrapper_dependencies.pri)

CONFIG(debug, debug|release) : LIBS += -ladinterfaced
CONFIG(release, debug|release) : LIBS += -ladinterface
INCLUDEPATH *= $$OUT_PWD/..

message( "LIBS " $$LIBS )

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
