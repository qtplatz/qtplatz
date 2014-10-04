#version check qt
contains(QT_VERSION, ^4\\.[0-7]\\..*) {
    message("Cannot build Qt Creator with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.0.")
}
cache()

include(qtplatz.pri)
include(src/version.pri)

TEMPLATE  = subdirs
CONFIG   += ordered
INCLUDEPATH += /usr/local/include

SUBDIRS = src \
    share \
    contrib

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
