#-------------------------------------------------
#
# Project created by QtCreator 2011-03-13T07:04:30
#
#-------------------------------------------------

QT       -= core gui

TARGET = adfs
TEMPLATE = lib
CONFIG += staticlib
include(../../qtplatz_library.pri)
include(../../boost.pri)

SOURCES += adfs.cpp \
    apiposix.cpp \
    apiwin32.cpp \
    attributes.cpp \
    cpio.cpp \
    filesystem.cpp \
    folder.cpp \
    folium.cpp \
    portfolio.cpp \
    sqlite.cpp \
    sqlite3.c

HEADERS += adfs.h \
    apiposix.h \
    apiwin32.h \
    attributes.h \
    cpio.h \
    filesystem.h \
    folder.h \
    folium.h \
    portfolio.h \
    sqlite.h \
    sqlite3.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
