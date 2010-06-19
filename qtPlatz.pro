#version check qt
contains(QT_VERSION, ^4\.[0-5]\..*) {
    message("Cannot build Qt Creator with Qt version $${QT_VERSION}.")
    error("Use at least Qt 4.6.")
}

include(qtPlatz.pri)

TEMPLATE  = subdirs
CONFIG   += ordered
INCLUDEPATH += /usr/local/include

SUBDIRS = src share
unix:!macx:!isEmpty(copydata):SUBDIRS += bin
