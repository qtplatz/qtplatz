#-------------------------------------------------
#
# Project created by QtCreator 2010-12-28T13:40:17
#
#-------------------------------------------------

QT       -= core gui

TARGET = chromatography
TEMPLATE = lib
CONFIG += staticlib

SOURCES += chromatography.cpp \
    theoreticalplates.cpp \
    stack.cpp \
    peakmoment.cpp \
    averager.cpp \
    noisefilter.cpp \
    peakfind.cpp \
    lsq.cpp \
    integrator.cpp

HEADERS += chromatography.h \
    theoreticalplates.h \
    stack.h \
    peakmoment.h \
    averager.h \
    noisefilter.h \
    peakfind.h \
    lsq.h \
    integrator.h
