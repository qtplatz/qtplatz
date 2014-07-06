#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T23:01:03
#
#-------------------------------------------------

QT       -= core gui

TARGET = adlog
TEMPLATE = lib

include(../../qtplatzlibrary.pri)
include(../../boost.pri)

LIBS += -l$$qtLibraryTarget(adportable)
!win32: { LIBS += -lboost_system -lboost_filesystem -lboost_date_time }

DEFINES += ADLOG_LIBRARY

SOURCES += adlog.cpp \
        logger.cpp \
        logging_handler.cpp

HEADERS += adlog.hpp\
        adlog_global.hpp \
        logger.hpp \
        logging_handler.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
