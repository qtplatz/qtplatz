#-------------------------------------------------
#
# Project created by QtCreator 2011-03-13T07:04:30
#
#-------------------------------------------------

QT       -= core gui

TARGET = adfs
TEMPLATE = lib
CONFIG += staticlib

SOURCES += adfs.cpp \
    adsqlite.cpp \
    sqlite3.c \
    apiposix.cpp \
    apiwin32.cpp \
    constants.cpp \
    filesystem.cpp \
    folder.cpp \
    folium.cpp \
        node.cpp \
    portfolio.cpp \
    node.cpp

HEADERS += adfs.h \
    adsqlite.h \
        sqlite3.h \
    apiwin32.h \
    apiposix.h \
    constants.h \
    filesystem.h \
    folder.h \
    folium.h \
    portfolio.h \
    node.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
