#-------------------------------------------------
#
# Project created by QtCreator 2010-07-30T12:33:13
#
#-------------------------------------------------

QT       -= gui

TARGET = adcontroller
TEMPLATE = lib

include(../../adplugin.pri)
include(../../boost.pri)
include(../../ace_tao.pri)
INCLUDEPATH *= $$OUT_PWD/../../libs

!win32: QMAKE_CXXFLAGS *= -std=c++11

win32: DEFINES += _SCL_SECURE_NO_WARNINGS

LIBS += -l$$qtLibraryTarget(adinterface)
LIBS += \
     -l$$qtLibraryTarget(adfs) \
     -l$$qtLibraryTarget(adportable) \
     -l$$qtLibraryTarget(acewrapper) \
     -l$$qtLibraryTarget(adinterface) \
     -l$$qtLibraryTarget(adplugin) \
     -l$$qtLibraryTarget(xmlparser)

win32 {
  LIBS += -l$$qtLibraryTarget(TAO_Utils) \
          -l$$qtLibraryTarget(TAO_PI) \
          -l$$qtLibraryTarget(TAO_PortableServer) \
          -l$$qtLibraryTarget(TAO_AnyTypeCode) \
          -l$$qtLibraryTarget(TAO) \
          -l$$qtLibraryTarget(ACE)
} else {
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem -lboost_thread -lboost_serialization
  LIBS += -lTAO_Utils \
          -lTAO_PI \
          -lTAO_PortableServer \
          -lTAO_AnyTypeCode \
          -lTAO \
          -lACE \
          -ldl
  linux-*: LIBS += -lrt
}

DEFINES += ADCONTROLLER_LIBRARY

SOURCES += adcontroller.cpp \
        cache.cpp \
        fileio.cpp \
        iproxy.cpp \
        manager_i.cpp \
        observer_i.cpp \
        oproxy.cpp \
        receiver_i.cpp \
        sampleprocessor.cpp \
        session_i.cpp \
        task.cpp \
        logging.cpp

HEADERS += adcontroller.hpp \
        adcontroller_global.h \
        cache.hpp \
        fileio.hpp \
        iproxy.hpp \
        manager_i.hpp \
        observer_i.hpp \
        oproxy.hpp \
        receiver_i.hpp \
        sampleprocessor.hpp \
        session_i.hpp \
        task.hpp \
        logging.hpp


