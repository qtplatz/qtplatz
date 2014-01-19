#-------------------------------------------------
#
# Project created by QtCreator 2011-04-10T07:13:16
#
#-------------------------------------------------

QT       += gui
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

TARGET = adwplot
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatzstaticlib.pri)
include(../../boost.pri)
include(../../qwt.pri)
INCLUDEPATH += ..

#message("INCLUDE=" $$INCLUDEPATH)

SOURCES += adwplot.cpp \
    annotation.cpp \
    annotations.cpp \
    axis.cpp \
    baseline.cpp \
    chromatogramwidget.cpp \
    dataplot.cpp \
    peak.cpp \
    plotcurve.cpp \
    seriesdata.cpp \
    spectrumwidget.cpp \
    tracewidget.cpp \
    trace.cpp \
    traces.cpp \
    zoomer.cpp \
    picker.cpp \
    panner.cpp \
    spectrogramwidget.cpp \
    spectrogramdata.cpp

HEADERS += adwplot.hpp \
    annotation.hpp \
    annotations.hpp \
    axis.hpp \
    baseline.hpp \
    chromatogramwidget.hpp \
    dataplot.hpp \
    peak.hpp \
    plotcurve.hpp \
    seriesdata.hpp \
    spectrumwidget.hpp \
    tracewidget.hpp \
    trace.hpp \
    traces.hpp \
    zoomer.hpp \
    picker.hpp \
    panner.hpp \
    spectrogramwidget.hpp \
    spectrogramdata.hpp

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
