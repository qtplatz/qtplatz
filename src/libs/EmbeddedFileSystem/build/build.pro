win32:TEMPLATE	= vclib
unix:TEMPLATE = lib
macx:TEMPLATE = lib
LANGUAGE	= C++

CONFIG	+= warn_on 

unix:CONFIG += staticlib
#unix:CONFIG += shared dll
win32:CONFIG += staticlib

win32:DEFINES += _CRT_SECURE_NO_DEPRECATE

QT      -= gui

ROOT_DIR = $$PWD
CONFIG += debug

include(../src/__build__.pri)
include(../jstreams/__build__.pri)
include(../btree/__build__.pri)

TARGET = Efs

DESTDIR = $$ROOT_DIR/dist_release/
OBJECTS_DIR = $$ROOT_DIR/obj/release/$$TARGET

CONFIG(debug, debug|release) {
 DESTDIR = $$ROOT_DIR/dist_debug/
 OBJECTS_DIR = $$ROOT_DIR/obj/debug/$$TARGET
}

#unix:LIBS       += -lGStreams -L$$DESTDIR
#win32:LIBS      += $$DESTDIR/GStreams.lib
