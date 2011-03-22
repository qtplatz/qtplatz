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
    constants.cpp \
    filesystem.cpp \
    apiwin32.cpp \
    adsqlite.cpp \
    folder.cpp \
    folium.cpp \
    portfolio.cpp \
    portfolioimpl.cpp \
    apiposix.cpp \
    node.cpp

HEADERS += adfs.h \
    constants.h \
    filesystem.h \
    apiwin32.h \
    adsqlite.h \
    folder.h \
    folium.h \
    portfolio.h \
    portfolioimpl.h \
    apiposix.h \
    node.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
