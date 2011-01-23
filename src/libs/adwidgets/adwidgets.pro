#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T22:32:02
#
#-------------------------------------------------

QT       += xml

TARGET = adwidgets
TEMPLATE = lib
CONFIG += staticlib qaxcontainer
INCLUDEPATH += $$(SATOOLS_ROOT)/bin

#DESTDIR = $$IDE_LIBRARY_PATH
include(../../adilibrary.pri)

SOURCES +=  dataplot.cpp \
    annotations.cpp \
    annotation.cpp \
    axis.cpp \
    baselines.cpp \
    baseline.cpp \
    colors.cpp \
    chromatogramwidget.cpp \
    dataplotimpl.cpp \
    dataplotwidget.cpp \
    filledranges.cpp \
    font.cpp \
    fractions.cpp \
    fraction.cpp \
    legend.cpp \
    markers.cpp \
    marker.cpp \
    peaks.cpp \
    peak.cpp \
    peakresultwidget.cpp \
    plotregion.cpp \
    ranges.cpp \
    spectrumwidget.cpp \
    tracewidget.cpp \
    traceaccessor.cpp \
    traces.cpp \
    trace.cpp \
    titles.cpp \
    title.cpp

HEADERS += dataplot.h \
    annotations.h \
    annotation.h \
    axis.h \
    baselines.h \
    baseline.h \
    colors.h \
    chromatogramwidget.h \
    dataplotimpl.h \
    dataplotwidget.h \
    filledranges.h \
    font.h \
    fractions.h \
    fraction.h \
    legend.h \
    markers.h \
    marker.h \
    peaks.h \
    peak.h \
    peakresultwidget.h \
    plotregion.h \
    ranges.h \
    spectrumwidget.h \
    tracewidget.h \
    traceaccessor.h \
    traces.h \
    trace.h \
    titles.h \
    title.h

include(../../boost.pri)

OTHER_FILES += \
    adwidgets.pri \
    boost.pri
