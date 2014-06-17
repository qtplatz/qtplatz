#-------------------------------------------------
#
# Project created by QtCreator 2010-06-27T16:23:07
#
#-------------------------------------------------

QT       -= gui

TARGET = acewrapper
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatzstaticlib.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

win32: QMAKE_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS

INCLUDEPATH *= $$OUT_PWD/..

SOURCES += constants.cpp \
           input_buffer.cpp \
           ifconfig.cpp \
           ifconfig_windows.cpp \
           iorsender.cpp \
           iorquery.cpp
           
HEADERS += constants.hpp \
           input_buffer.hpp \
           orbservant.hpp \
           ifconfig.hpp \
           ifconfig_macosx.hpp \
           ifconfig_linux.hpp \
           ifconfig_windows.hpp \
           iorsender.hpp \
           iorquery.hpp

OTHER_FILES += \
    acewrapper.pri



