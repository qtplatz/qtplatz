#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:46:27
#
#-------------------------------------------------

QT       += gui svg

TARGET = acquire
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include(../../qtplatzplugin.pri)
include(../../boost.pri)
include(../../ace_tao.pri)
include(../../qwt.pri)

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adutils) -l$$qtLibraryTarget(adinterface) \
    -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adwplot) \
    -l$$qtLibraryTarget(acewrapper) -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(adplugin) \
    -l$$qtLibraryTarget(adextension)

!win32 {
  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
  LIBS += -lboost_date_time
}
linux-*: LIBS += -lqwt # order matter on linux

include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ACQUIRE_LIBRARY

SOURCES += acquire.cpp \
	acquireactions.cpp \
	acquiremode.cpp \
	acquireplugin.cpp \
	acquireuimanager.cpp

HEADERS +=  acquire_global.h \
	acquire.hpp \
	acquireactions.hpp \
	acquiremode.hpp \
	acquireplugin.hpp \
	acquireuimanager.hpp \
	constants.hpp

OTHER_FILES += acquire.pluginspec \
    acquire.config.xml \
    acquire_dependencies.pri

RESOURCES += \
    acquire.qrc
