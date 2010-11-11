#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

TARGET = qtwidgets
TEMPLATE = lib
# INCLUDEPATH += ../../libs ../../plugins
include(../../adilibrary.pri)
include(../../boost.pri)
PROVIDER = ScienceLiaison
include(../../adplugin.pri)
INCLUDEPATH *= $(ACE_ROOT) $(TAO_ROOT)


DEFINES += QTWIDGETS_LIBRARY

SOURCES += qtwidgets.cpp \
    logwidget.cpp \
    factory.cpp \
    sequencewidget.cpp \
    sequences.cpp \
    centroidform.cpp \
    elementalcompositionform.cpp \
    mscalibrationform.cpp \
    isotopeform.cpp \
    targetingform.cpp \
    mslockform.cpp \
    chromatographicpeakform.cpp \
    peakidtableform.cpp \
    reportform.cpp \
    centroidmodel.cpp \
    qcomboboxdelegate.cpp

HEADERS += qtwidgets.h\
        qtwidgets_global.h \
    logwidget.h \
    factory.h \
    sequencewidget.h \
    sequences.h \
    centroidform.h \
    elementalcompositionform.h \
    mscalibrationform.h \
    isotopeform.h \
    targetingform.h \
    mslockform.h \
    chromatographicpeakform.h \
    peakidtableform.h \
    reportform.h \
    centroidmodel.h \
    qcomboboxdelegate.h

FORMS += \
    logwidget.ui \
    sequencewidget.ui \
    sequencesform.ui \
    centroidform.ui \
    elementalcompositionform.ui \
    mscalibrationform.ui \
    isotopeform.ui \
    targetingform.ui \
    mslockform.ui \
    chromatographicpeakform.ui \
    peakidtableform.ui \
    reportform.ui
