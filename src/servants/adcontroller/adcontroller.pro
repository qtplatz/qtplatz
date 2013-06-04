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
LIBS *= -l$$qtLibraryTarget(adinterface)
INCLUDEPATH *= $$OUT_PWD/../../libs

win32: DEFINES += _SCL_SECURE_NO_WARNINGS

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
  LIBS += -lboost_thread -lboost_date_time -lboost_system -lboost_filesystem -lboost_thread
  linux-*: LIBS += -lrt
}

LIBS += \
     -l$$qtLibraryTarget(acewrapper) \
     -l$$qtLibraryTarget(adinterface) \
     -l$$qtLibraryTarget(adportable) \
     -l$$qtLibraryTarget(adplugin) \
     -l$$qtLibraryTarget(xmlparser)

DEFINES += ADCONTROLLER_LIBRARY

SOURCES += adcontroller.cpp \
        iproxy.cpp \
        manager_i.cpp \
        mcast_handler.cpp \
        message.cpp \
        observer_i.cpp \
        oproxy.cpp \
        receiver_i.cpp \
        session_i.cpp \
        signal_handler.cpp \
        cache.cpp \
        task.cpp \
        taskmanager.cpp \
        logging.cpp

HEADERS += adcontroller.hpp \
        adcontroller_global.h \
        constants.hpp \
        iproxy.hpp \
        manager_i.hpp \
        marshal.hpp \
        mcast_handler.hpp \
        message.hpp \
        observer_i.hpp \
        oproxy.hpp \
        receiver_i.hpp \
        session_i.hpp \
        signal_handler.hpp \
        cache.hpp \
        task.hpp \
        taskmanager.hpp \
        logging.hpp












