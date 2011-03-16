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
    win32api.cpp \
    posixapi.cpp

HEADERS += adfs.h \
    superblock.h \
    constants.h \
    inode.h \
    operations.h \
    filesystem.h \
    win32api.h \
    posixapi.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
