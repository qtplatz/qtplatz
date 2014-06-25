#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T11:29:18
#
#-------------------------------------------------

QT       -= gui

TARGET = adbroker
TEMPLATE = lib

!win32: QMAKE_CXXFLAGS *= -std=c++11

include(../../adplugin.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

INCLUDEPATH *= $$OUT_PWD/../../libs

LIBS += -l$$qtLibraryTarget(adinterface) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(acewrapper) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adorbmgr) \
        -l$$qtLibraryTarget(adlog)

win32 {
  LIBS += -l$$qtLibraryTarget(TAO_Utils) \
          -l$$qtLibraryTarget(TAO_PI) \
          -l$$qtLibraryTarget(TAO_PortableServer) \
          -l$$qtLibraryTarget(TAO_AnyTypeCode) \
          -l$$qtLibraryTarget(TAO) \
          -l$$qtLibraryTarget(ACE)
} else {
  LIBS += -lTAO_Utils \
          -lTAO_PI \
          -lTAO_PortableServer \
          -lTAO_AnyTypeCode \
          -lTAO \
          -lACE
  LIBS += -lboost_system -lboost_filesystem -lboost_serialization -lboost_date_time -ldl
  linux-*: LIBS += -lrt
}


DEFINES += ADBROKER_LIBRARY

SOURCES += adbroker.cpp \
    brokerconfig.cpp \
    brokermanager.cpp \
    brokersession.cpp \
    brokertoken.cpp \
    logger_i.cpp \
    manager_i.cpp \
    session_i.cpp \
    task.cpp \
    objectdiscovery.cpp \
    orbbroker.cpp

HEADERS += adbroker.hpp \
    adbroker_global.h \
    brokermanager.hpp \
    brokerconfig.hpp \
    brokermanager.hpp \
    brokersession.hpp \
    brokertoken.hpp \
    logger_i.hpp \
    manager_i.hpp \
    session_i.hpp \
    task.hpp \
    objectdiscovery.hpp \
    orbbroker.hpp

OTHER_FILES += \
    adbroker.pri \
    adbroker_dependencies.pri


