#-------------------------------------------------
#
# Project created by QtCreator 2013-06-05T09:47:05
#
#-------------------------------------------------

QT       -= core gui

TARGET = adorbmgr
TEMPLATE = lib
include(../../qtplatzlibrary.pri)
include(../../ace_tao.pri)
include(../../boost.pri)

DEFINES += ADORBMGR_LIBRARY

SOURCES += adorbmgr.cpp \
           orbmgr.cpp

HEADERS += adorbmgr.h\
        adorbmgr_global.h \
        orbmgr.hpp

LIBS += -l$$qtLibraryTarget(acewrapper) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser)

!macx {
  LIBS += -l$$qtLibraryTarget( TAO_Utils ) \
          -l$$qtLibraryTarget( TAO_PortableServer ) \
          -l$$qtLibraryTarget( TAO_AnyTypeCode ) \
          -l$$qtLibraryTarget( TAO ) \
          -l$$qtLibraryTarget( ACE )
} else {
  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
}

!win32: LIBS += -lboost_thread -lboost_system

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
