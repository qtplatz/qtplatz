#-------------------------------------------------
#
# Project created by QtCreator 2013-06-05T09:47:05
#
#-------------------------------------------------

QT       -= core gui

TARGET = adorbmgr
TEMPLATE = lib
include(../../qtplatzlibrary.pri)
include(../../ace_tao.pri)
include(../../boost.pri)

DEFINES += ADORBMGR_LIBRARY

SOURCES += adorbmgr.cpp

HEADERS += adorbmgr.h\
        adorbmgr_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
