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
    axis.cpp \
    traces.cpp \
    trace.cpp \
    titles.cpp \
    title.cpp \
    colors.cpp \
    legend.cpp \
    plotregion.cpp \
    annotations.cpp \
    annotation.cpp \
    fractions.cpp \
    fraction.cpp \
    markers.cpp \
    marker.cpp \
    peaks.cpp \
    peak.cpp \
    baselines.cpp \
    baseline.cpp \
    filledranges.cpp \
    ranges.cpp \
    font.cpp \
    dataplotimpl.cpp \
    dataplotwidget.cpp \
    chromatogramwidget.cpp \
    spectrumwidget.cpp \
    tracewidget.cpp

HEADERS += dataplot.h \
    axis.h \
    traces.h \
    trace.h \
    titles.h \
    title.h \
    colors.h \
    legend.h \
    import_sagraphics.h \
    plotregion.h \
    annotations.h \
    annotation.h \
    fractions.h \
    fraction.h \
    markers.h \
    marker.h \
    peaks.h \
    peak.h \
    baselines.h \
    baseline.h \
    filledranges.h \
    ranges.h \
    font.h \
    dataplotimpl.h \
    dataplotwidget.h \
    chromatogramwidget.h \
    spectrumwidget.h \
    tracewidget.h

include(../../boost.pri)

OTHER_FILES += \
    adwidgets.pri \
    boost.pri
