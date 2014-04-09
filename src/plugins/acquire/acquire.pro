#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:46:27
#
#-------------------------------------------------

QT       += gui svg printsupport

TARGET = acquire
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include(../../qtplatzplugin.pri)
include(../../boost.pri)
include(../../ace_tao.pri)
include(../../qwt.pri)

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(adinterface) \
        -l$$qtLibraryTarget(adwplot) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(acewrapper) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser) \
        -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adorbmgr)

win32 {
  LIBS += -l$$qtLibraryTarget( TAO_Utils ) \
          -l$$qtLibraryTarget( TAO_PortableServer ) \
          -l$$qtLibraryTarget( TAO_AnyTypeCode ) \
          -l$$qtLibraryTarget( TAO ) \
          -l$$qtLibraryTarget( ACE )
} else {
  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
  LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}
linux-*: LIBS += -lqwt # order matter on linux

include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ACQUIRE_LIBRARY

SOURCES += \
	acquiremode.cpp \
	acquireplugin.cpp \
	mainwindow.cpp \
    qbroker.cpp

HEADERS +=  acquire_global.h \
	acquiremode.hpp \
	acquireplugin.hpp \
	mainwindow.hpp \
	constants.hpp \
    qbroker.hpp

OTHER_FILES += acquire.pluginspec \
    acquire.config.xml \
    acquire_dependencies.pri

RESOURCES += \
    acquire.qrc
