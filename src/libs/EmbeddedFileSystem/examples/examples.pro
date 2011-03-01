win32:TEMPLATE	= vcapp
unix:TEMPLATE = app
macx:TEMPLATE = app
LANGUAGE	= C++

CONFIG	+= warn_on 

unix:CONFIG += console
win32:CONFIG += console

ROOT_DIR = $$PWD
CONFIG += debug

QT      -= gui

include(../src/__build__.pri)
include(../jstreams/__build__.pri)
include(../btree/__build__.pri)

TARGET = EfsTests

SOURCES += main.cpp

DESTDIR = $$ROOT_DIR/dist_release/
OBJECTS_DIR = $$ROOT_DIR/test_obj/release/$$TARGET

CONFIG(debug, debug|release) {
 DESTDIR = $$ROOT_DIR/dist_debug/
 OBJECTS_DIR = $$ROOT_DIR/test_obj/debug/$$TARGET
}
CONFIG(release) {
 DEFINES += QT_NO_DEBUG_OUTPUT
}

#unix:LIBS       += -lGStreams -L$$DESTDIR

