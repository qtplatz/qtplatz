#-------------------------------------------------
#
# Project created by QtCreator 2011-02-12T16:13:21
#
#-------------------------------------------------

QT -= gui core 

TARGET = addatafile
TEMPLATE = lib

DEFINES += ADDATAFILE_LIBRARY

include(../../boost.pri)
include(../../qtplatz_servant.pri)
INCLUDEPATH += ../

LIBS += -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adportable) \
    -l$$qtLibraryTarget(adutils) \
    -l$$qtLibraryTarget(portfolio) \
    -l$$qtLibraryTarget(xmlparser) \
    -l$$qtLibraryTarget(adfs)

!win32 {
  LIBS += -lboost_system -lboost_filesystem -lboost_serialization -lboost_date_time
}

SOURCES += addatafile.cpp \
    datafile.cpp \
    datafile_factory.cpp \
    copyin_visitor.cpp

HEADERS += addatafile.hpp \
        addatafile_global.h \
    datafile.hpp \
    datafile_factory.hpp \
    copyin_visitor.hpp
