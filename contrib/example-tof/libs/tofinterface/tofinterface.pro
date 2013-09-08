#-------------------------------------------------
#
# Project created by QtCreator 2012-11-15T10:35:19
#
#-------------------------------------------------

QT       -= core gui

TARGET = tofinterface
TEMPLATE = lib
CONFIG += staticlib

include( ../../tofstaticlib.pri )
include( ../../boost.pri )
include( ../../ace_tao.pri )

INCLUDEPATH += $${QTPLATZ_SOURCE_TREE}/src/libs ..
IDL_INCLUDES += -I$${QTPLATZ_SOURCE_TREE}/src/libs

IDLFILES = constants.idl \
           signal.idl \
           sio.idl \
           tof.idl \

include( ../../tao_idl.pri )

!win32: QMAKE_CXXFLAGS += -std=c++0x -Wno-unused-parameter
win32:  QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS
LIBS += -l$$qtLibraryTarget( adportable )

SOURCES += tofinterface.cpp \
           tofdeviceid.cpp \
           tofstaticsetpts.cpp \
           tofstaticacts.cpp \
           tofmasscommand.cpp \
           tofdata.cpp \
           tofprocessed.cpp \
           tofacqmethod.cpp \
           rawxfer.cpp \
           serializer.cpp

HEADERS += tofinterface.hpp \
           tofdeviceid.hpp \
           protocolids.hpp \
           tofstaticsetpts.hpp \
           tofstaticacts.hpp \
           tofmasscommand.hpp \
           tofdata.hpp \
           tofprocessed.hpp \
           tofacqmethod.hpp \
           tracemetadata.hpp \
           cstdint.hpp \
           rawxfer.hpp \
           dma_type.hpp \
           fpgaio.hpp \
           serializer.hpp \
           tofdef.hpp

OTHER_FILES += \
    tofmethod.idl \
    tofsignal.idl

