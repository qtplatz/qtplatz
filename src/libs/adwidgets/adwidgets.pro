#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T22:32:02
#
#-------------------------------------------------

QT       += xml

TARGET = adwidgets
TEMPLATE = lib
CONFIG += staticlib qaxcontainer

#DESTDIR = $$IDE_LIBRARY_PATH
include(../../qtPlatzlibrary.pri)

SOURCES +=  dataplot.cpp \
    axis.cpp \
    traces.cpp \
    trace.cpp \
    titles.cpp \
    title.cpp \
    colors.cpp \
    legend.cpp \
    plotregion.cpp

HEADERS += dataplot.h \
    axis.h \
    traces.h \
    trace.h \
    titles.h \
    title.h \
    colors.h \
    legend.h \
    import_sagraphics.h \
    plotregion.h

include(../../boost.pri)

OTHER_FILES += \
    adwidgets.pri \
    boost.pri
