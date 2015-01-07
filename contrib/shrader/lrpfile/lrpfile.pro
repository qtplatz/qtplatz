#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T12:50:00
#
#-------------------------------------------------

QT       -= gui

include(../../contrib.pri)
include(../../../src/boost.pri)
include(../../../src/qtplatzstaticlib.pri)

PROVIDER = MS-Cheminfomatics

TEMPLATE = lib
CONFIG  += staticlib
TARGET = $$qtLibraryTarget(lrpfile)

INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src

DEFINES += SPCFILE_LIBRARY

!win32 {
    LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}

SOURCES += lrpfile.cpp \
           lrpheader.cpp \
           lrphead2.cpp \
           lrphead3.cpp \
           instsetup.cpp \
           lrpcalib.cpp \
           simions.cpp \
           lrptic.cpp \
           msdata.cpp

HEADERS += \
        lrpfile.hpp \
        lrpheader.hpp \
        lrphead2.hpp \
        lrphead3.hpp \
        instsetup.hpp \
        lrpcalib.hpp \
        simions.hpp \
        lrptic.hpp \
        msdata.hpp
