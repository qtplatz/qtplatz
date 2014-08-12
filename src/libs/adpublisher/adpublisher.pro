#-------------------------------------------------
#
# Project created by QtCreator 2014-08-12T18:01:52
#
#-------------------------------------------------

QT       += sql svg xml

QT       -= gui

TARGET = adpublisher
TEMPLATE = lib

INCLUDEPATH += ../../libs
include(../../boost.pri)
include(../../qtplatzlibrary.pri)

DEFINES += ADPUBLISHER_LIBRARY

SOURCES += adpublisher.cpp \
           document.cpp

HEADERS += adpublisher.hpp\
           adpublisher_global.hpp \
           document.hpp


LIBS *= -L$$IDE_LIBRARY_PATH 
LIBS += -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(adportable)

!win32: LIBS += -lboost_date_time -lboost_system

unix {
    target.path = /usr/lib
    INSTALLS += target
}
