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
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(xmlparser) \
        -l$$qtLibraryTarget(Core)

!win32 {
  LIBS *= -lboost_serialization -lboost_date_time -lboost_filesystem -lboost_system
}

DEFINES += SERVANT_LIBRARY

SOURCES += outputwindow.cpp \
        servant.cpp \
        servantmode.cpp \
        servantplugin.cpp \
    logger.cpp

HEADERS += servant_global.h \
        outputwindow.hpp \
        servant.hpp \
        servantmode.hpp \
        servantplugin.hpp \
    logger.hpp

OTHER_FILES += \
    servant.pluginspec \
    servant.config

