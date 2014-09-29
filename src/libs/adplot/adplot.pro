#-------------------------------------------------
#
# Project created by QtCreator 2011-04-10T07:13:16
#
#-------------------------------------------------

QT       += gui svg printsupport
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

TARGET = adplot
TEMPLATE = lib

DEFINES += ADPLOT_LIBRARY

include(../../qtplatzlibrary.pri)
include(../../boost.pri)
include(../../qwt.pri)
INCLUDEPATH += ..

LIBS += -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper)

SOURCES += annotation.cpp \
           annotations.cpp \
           axis.cpp \
           baseline.cpp \
           chromatogramwidget.cpp \
           plot.cpp \
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
           spectrogramdata.cpp \
           peakmarker.cpp \
           plot_stderror.cpp \
           adplotcurve.cpp \
    timingchart.cpp

HEADERS += annotation.hpp \
           annotations.hpp \
           axis.hpp \
           baseline.hpp \
           chromatogramwidget.hpp \
           plot.hpp \
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
           spectrogramdata.hpp \
           peakmarker.hpp \
           plot_stderror.hpp \
           adplotcurve.hpp \
    timingchart.hpp


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
