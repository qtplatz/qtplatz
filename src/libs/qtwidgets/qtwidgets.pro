#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

QT += core gui declarative

TARGET = qtwidgets
TEMPLATE = lib
PROVIDER = ScienceLiaison

include(../../qtplatz_servant.pri)
include(../../boost.pri)
LIBS += -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adplugin) -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser)

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
        peakresultwidget.cpp \
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
        treeitem.cpp \
    processmethodview.cpp \
    centroidmethodmodel.cpp \
    isotopemethodmodel.cpp \
    chemicalformula.cpp


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
        peakresultwidget.hpp \
        qtwidgets.hpp \
        reportdelegate.hpp \
        reportform.hpp \
        sequencesform.hpp \
        sequencesmodel.hpp \
        sequencewidget.hpp \
        standarditemhelper.hpp \
        standardmodel.hpp \
        targetingdelegate.hpp \
        targetingform.hpp \
        treeitem.hpp \
    processmethodview.hpp \
    centroidmethodmodel.hpp \
    isotopemethodmodel.hpp \
    chemicalformula.hpp

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

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    qml/ProcessMethodEditor.qml \
    qml/content/BusyIndicator.qml \
    qml/content/CategoryDelegate.qml \
    qml/content/ScrollBar.qml \
    qml/content/EditCentroidMethod.qml \
    qml/content/EditElementalCompMethod.qml \
    qml/content/EditIntegrationMethod.qml \
    qml/content/EditIsotopeMethod.qml \
    qml/content/EditLockMassMethod.qml \
    qml/content/EditMSCalibMethod.qml \
    qml/content/EditPeakIDTable.qml \
    qml/content/EditReportMethod.qml \
    qml/content/EditTargetMethod.qml \
    qml/content/ScanType.qml \
    qml/content/CaptionText.qml \
    qml/content/TextInputBox.qml \
    qml/content/ScanTypeDetails.qml \
    qml/content/MethodEditDelegate.qml \
    qml/content/TitleText.qml
