#-------------------------------------------------
#
# Project created by QtCreator 2010-08-11T12:03:35
#
#-------------------------------------------------

PROVIDER = MS-Cheminformatics

include(../../qtplatzplugin.pri)
include(../../ace_tao.pri)
include(../../boost.pri)

LIBS += -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adinterface) -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adplugin) \
    -l$$qtLibraryTarget(qtwrapper) -l$$qtLibraryTarget(adbroker) \
    -l$$qtLibraryTarget(adextension) \
    -l$$qtLibraryTarget(xmlparser) \
    -l$$qtLibraryTarget(adorbmgr) \
    -l$$qtLibraryTarget(Core)

!win32 {
  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
  LIBS *= -lboost_serialization -lboost_date_time -lboost_filesystem -lboost_system
}

DEFINES += SERVANT_LIBRARY

#  orbservantmanager.cpp --> to be deleted
#  orbservantmanager.hpp --> to be deleted

SOURCES += logger.cpp \
        outputwindow.cpp \
        servant.cpp \
        servantmode.cpp \
        servantplugin.cpp \
        servantpluginimpl.cpp

HEADERS += servant_global.h \
        logger.hpp \
        outputwindow.hpp \
        servant.hpp \
        servantmode.hpp \
        servantplugin.hpp \
        servantpluginimpl.hpp

OTHER_FILES += \
    servant.pluginspec \
    servant.config

