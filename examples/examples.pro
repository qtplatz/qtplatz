#version check qt
contains(QT_VERSION, ^4\\.[0-6]\\..*) {
    message("Cannot build Qt Creator with Qt version $${QT_VERSION}.")
    error("Use at least Qt 4.7.")
}

#include(qtplatz.pri)

TEMPLATE  = subdirs
CONFIG   += ordered
INCLUDEPATH += /usr/local/include

SUBDIRS = device_emulators \
        bcastaddr \
        formula_parser \
        sdfile_parser \
    isotope

unix:!macx:!isEmpty(copydata):SUBDIRS += bin

