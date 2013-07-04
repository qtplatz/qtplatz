#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T11:29:18
#
#-------------------------------------------------

QT       -= gui

!win32: QMAKE_CXXFLAGS *= -std=c++11

include(../../tofadplugin.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

INCLUDEPATH *= $$OUT_PWD/../../libs

LIBS += -l$$qtLibraryTarget(tofinterface) \
    -l$$qtLibraryTarget(adinterface) \
    -l$$qtLibraryTarget(adportable) \
    -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(portfolio) \
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
  LIBS += -lTAO_Utils \
          -lTAO_PI \
          -lTAO_PortableServer \
          -lTAO_AnyTypeCode \
          -lTAO \
          -lACE
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem -lboost_thread -lboost_regex
  linux-*: LIBS += -lrt
}


DEFINES += TOFSERVANT_LIBRARY

SOURCES += tofmgr_i.cpp \
           tofservant.cpp \
           tofsession_i.cpp

HEADERS += tofservant_global.h \
           tofmgr_i.hpp \
           tofservant.hpp \
           tofsession_i.hpp

OTHER_FILES += \
    tofservent.pri \
    tofservant_dependencies.pri


