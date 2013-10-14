#-------------------------------------------------
#
# Project created by QtCreator 2011-02-12T16:13:21
#
#-------------------------------------------------

QT -= gui

TARGET = addatafile
TEMPLATE = lib

DEFINES += ADDATAFILE_LIBRARY

include(../../boost.pri)
include(../../adplugin.pri)
include(../../ace_tao.pri)

INCLUDEPATH += ../../libs

LIBS +=  -l$$qtLibraryTarget(adplugin) \
         -l$$qtLibraryTarget(acewrapper) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(xmlparser) \
        -l$$qtLibraryTarget(adplugin)
        
!win32 {
  LIBS += -lboost_system -lboost_filesystem -lboost_serialization -lboost_date_time -ldl
}

SOURCES += addatafile.cpp \
    datafile.cpp \
    datafile_factory.cpp \
    rawdata.cpp \
    cpio.cpp

HEADERS += addatafile.hpp \
        addatafile_global.h \
    datafile.hpp \
    datafile_factory.hpp \
    rawdata.hpp \
    cpio.hpp
