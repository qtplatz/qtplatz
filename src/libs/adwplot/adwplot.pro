#-------------------------------------------------
#
# Project created by QtCreator 2011-04-10T07:13:16
#
#-------------------------------------------------

QT       -= gui
TARGET = adwplot
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatz_library.pri)
include(../../boost.pri)
INCLUDEPATH += $$(QWT)/include $$(QTDIR)/include/Qtcore $$(QTDIR)/include/QtGui
INCLUDEPATH += ..


# message("INCLUDE=" $$(INCLUDEPATH))


SOURCES += adwplot.cpp \
    annotation.cpp \
    annotations.cpp \
    axis.cpp \
    baseline.cpp \
    chromatogramwidget.hpp \
    dataplot.cpp \
    peak.cpp \
    plotcurve.cpp \
    plotpanner.cpp \
    plotpicker.cpp \
    seriesdata.cpp \
    spectrumwidget.cpp \
    trace.cpp \
    traces.cpp \
    zoomer.cpp

HEADERS += adwplot.hpp \
    annotation.hpp \
    annotations.hpp \
    axis.hpp \
    baseline.hpp \
    chromatogramwidget.hpp \
    dataplot.hpp \
    peak.hpp \
    plotcurve.hpp \
    plotpanner.hpp \
    plotpicker.hpp \
    seriesdata.hpp \
    spectrumwidget.hpp \
    trace.hpp \
    traces.hpp \
    zoomer.hpp

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
