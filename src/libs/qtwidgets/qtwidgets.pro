#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

TARGET = qtwidgets
TEMPLATE = lib
# INCLUDEPATH += ../../libs ../../plugins
include(../../qtplatz_lib_dynamic.pri)
include(../../boost.pri)
PROVIDER = ScienceLiaison
include(../../adplugin.pri)
INCLUDEPATH *= $(ACE_ROOT) $(TAO_ROOT)


DEFINES += QTWIDGETS_LIBRARY

SOURCES += centroiddelegate.cpp \
        centroidform.cpp \
        chromatographicpeakform.cpp \
        elementalcompositiondelegate.cpp \
        elementalcompositionform.cpp \
        factory.cpp \
        isotopedelegate.cpp \
        isotopeform.cpp \
        logwidget.cpp \
        mscalibratedelegate.cpp \
        mscalibrationform.cpp \
        mscalibsummarydelegate.cpp \
        mscalibsummarywidget.cpp \
        mslockdelegate.cpp \
        mslockform.cpp \
        peakidtableform.cpp \
        qtwidgets.cpp \
        reportdelegate.cpp \
        reportform.cpp \
        sequencesform.cpp \
        sequencesmodel.cpp \
        sequencewidget.cpp \
        standarditemhelper.cpp \
        standardmodel.cpp \
        targetingdelegate.cpp \
        targetingform.cpp \
        treeitem.cpp


HEADERS += centroiddelegate.h \
        centroidform.h \
        chromatographicpeakform.h \
        elementalcompositiondelegate.h \
        elementalcompositionform.h \
        factory.h \
        isotopedelegate.h \
        isotopeform.h \
        logwidget.h \
        mscalibratedelegate.h \
        mscalibrationform.h \
        mscalibsummarydelegate.h \
        mscalibsummarywidget.h \
        mslockdelegate.h \
        mslockform.h \
        peakidtableform.h \
        qtwidgets.h \
        reportdelegate.h \
        reportform.h \
        sequencesform.h \
        sequencesmodel.h \
        sequencewidget.h \
        standarditemhelper.h \
        standardmodel.h \
        targetingdelegate.h \
        targetingform.h \
        treeitem.h

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
