#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

TARGET = qtwidgets
TEMPLATE = lib
# INCLUDEPATH += ../../libs ../../plugins
PROVIDER = ScienceLiaison
include(../../qtplatz_library.pri)
include(../../boost.pri)
# pragma comment(lib, "adportabled.lib")
# pragma comment(lib, "adcontrolsd.lib")
# pragma comment(lib, "adplugind.lib")
# pragma comment(lib, "qtwrapperd.lib")
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


HEADERS += centroiddelegate.hpp \
        centroidform.hpp \
        chromatographicpeakform.hpp \
        elementalcompositiondelegate.hpp \
        elementalcompositionform.hpp \
        factory.hpp \
        isotopedelegate.hpp \
        isotopeform.hpp \
        logwidget.hpp \
        mscalibratedelegate.hpp \
        mscalibrationform.hpp \
        mscalibsummarydelegate.hpp \
        mscalibsummarywidget.hpp \
        mslockdelegate.hpp \
        mslockform.hpp \
        peakidtableform.hpp \
        qtwidgets.hpp \
        reportdelegate.hpp \
        reportform.hpp \
        sequencesform.hpp \
        sequencesmodel.hpp \
        sequencewidget.hpp \
        standardite.hppelper.hpp \
        standardmodel.hpp \
        targetingdelegate.hpp \
        targetingform.hpp \
        treeitem.hpp

FORMS += \
    centroidform.ui \
    chromatographicpeakform.ui \
    elementalcompositionform.ui \
    isotopeform.ui \
    logwidget.ui \
    mscalibrationform.ui \
    mslockform.ui \
    peakidtableform.ui \
    peakresulttable.ui \
    reportform.ui \
    sequencesform.ui \
    sequencewidget.ui \
    targetingform.ui
