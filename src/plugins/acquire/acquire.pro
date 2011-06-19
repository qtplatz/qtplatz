#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:46:27
#
#-------------------------------------------------

QT       += gui

TARGET = acquire
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)

include(../../libs/acewrapper/acewrapper_dependencies.pri)
include(../../boost.pri)
include(../../qwt.pri)

INCLUDEPATH *= $$OUT_PWD/../../libs ../../servants ../ $$(QWT)/include
#PRE_TARGETDEPS += acquire.pro

LIBS += -L$$IDE_PLUGIN_PATH/Nokia -L$$IDE_LIBRARY_PATH
LIBS += -l$$qtLibraryTarget(adcontroller) -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adutils) -l$$qtLibraryTarget(adinterface) \
    -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adwplot) \
    -l$$qtLibraryTarget(acewrapper) -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(adplugin)

LIBS += -lqwt
LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
LIBS += -lboost_date_time

include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ACQUIRE_LIBRARY

SOURCES += acquire.cpp \
	acquireactions.cpp \
	acquiremode.cpp \
	acquireplugin.cpp \
	acquireuimanager.cpp \
	session.cpp

HEADERS +=  acquire_global.h \
	acquire.hpp \
	acquireactions.hpp \
	acquiremode.hpp \
	acquireplugin.hpp \
	acquireuimanager.hpp \
	constants.hpp \
	session.hpp

OTHER_FILES += acquire.pluginspec \
    acquire.config.xml \
    acquire_dependencies.pri

RESOURCES += \
    acquire.qrc
