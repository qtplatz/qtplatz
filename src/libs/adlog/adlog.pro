#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T23:01:03
#
#-------------------------------------------------

QT       -= gui

TARGET = adlog
TEMPLATE = lib

include(../../qtplatzlibrary.pri)

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
