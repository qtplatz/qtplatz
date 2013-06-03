#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T06:44:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = adinterface
TEMPLATE = lib
CONFIG += staticlib
include(../../qtplatzstaticlib.pri)
include(../../boost.pri)
include(../../ace_tao.pri)
# include(adinterface_dependencies.pri)

IDLFILES += \
    brokerevent.idl \
    controlmethod.idl \
    controlserver.idl \
    eventlog.idl \
    global_constants.idl \
    instrument.idl \
    loghandler.idl \
    receiver.idl \
    samplebroker.idl \
    signalobserver.idl \
    broker.idl \
    brokerclient.idl

include( ../../tao_idl.pri )

SOURCES += interface.cpp \
        eventlog_helper.cpp \
    	controlmethodhelper.cpp \
        controlmethodaccessor.cpp

HEADERS += interface.hpp \
        eventlog_helper.hpp \
        controlmethodhelper.hpp \
        controlmethodaccessor.hpp

OTHER_FILES += \
    adinterface_dependencies.pri
