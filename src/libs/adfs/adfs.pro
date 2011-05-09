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

HEADERS += adfs.hpp \
    apiposix.hpp \
    apiwin32.hpp \
    attributes.hpp \
    cpio.hpp \
    filesystem.hpp \
    folder.hpp \
    folium.hpp \
    portfolio.hpp \
    sqlite.hpp \
    sqlite3.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
