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
    superblock.cpp \
    constants.cpp \
    inode.cpp \
    operations.cpp \
    filesystem.cpp \
    apiwin32.cpp \
    apiposix.cpp \
    adsqlite.cpp

HEADERS += adfs.h \
    superblock.h \
    constants.h \
    inode.h \
    operations.h \
    filesystem.h \
    apiwin32.h \
    apiposix.h \
    adsqlite.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
