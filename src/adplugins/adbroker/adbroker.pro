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
    -l$$qtLibraryTarget(adportable) \
    -l$$qtLibraryTarget(adfs) \
    -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(portfolio) \
    -l$$qtLibraryTarget(adplugin)


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
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem -lboost_thread -lboost_regex
  linux-*: LIBS += -lrt
}


DEFINES += ADBROKER_LIBRARY

SOURCES += adbroker.cpp \
    brokerconfig.cpp \
    brokermanager.cpp \
    brokersession.cpp \
    brokertoken.cpp \
    chemicalformula_i.cpp \
    logger_i.cpp \
    manager_i.cpp \
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
    session_i.hpp \
    task.hpp \
    objectdiscovery.hpp

OTHER_FILES += \
    adbroker.pri \
    adbroker_dependencies.pri


