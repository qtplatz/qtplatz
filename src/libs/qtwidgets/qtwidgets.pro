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

!win32 {
  LIBS *= -lboost_filesystem -lboost_system
}

INCLUDEPATH *= $(ACE_ROOT) $(TAO_ROOT)

DEFINES += QTWIDGETS_LIBRARY

SOURCES += centroiddelegate.cpp \
        centroidform.cpp \
        elementalcompositiondelegate.cpp \
        elementalcompositionform.cpp \
        factory.cpp \
        logwidget.cpp \
        mscalibratedelegate.cpp \
        mscalibrationform.cpp \
        mscalibsummarydelegate.cpp \
        mscalibsummarywidget.cpp \
        peakresultwidget.cpp \
        qtwidgets.cpp \
        sequencesform.cpp \
        sequencesmodel.cpp \
        sequencewidget.cpp \
        standarditemhelper.cpp \
        standardmodel.cpp \
        treeitem.cpp \
    processmethodview.cpp \
    centroidmethodmodel.cpp \
    isotopemethodmodel.cpp \
    chemicalformula.cpp \
    elementalcompmodel.cpp \
    mscalibratemodel.cpp \
    molwidget.cpp \
    isotopeform.cpp \
    isotopedelegate.cpp



HEADERS += centroiddelegate.hpp \
        centroidform.hpp \
        elementalcompositiondelegate.hpp \
        elementalcompositionform.hpp \
        factory.hpp \
        logwidget.hpp \
        mscalibratedelegate.hpp \
        mscalibrationform.hpp \
        mscalibsummarydelegate.hpp \
        mscalibsummarywidget.hpp \
        peakresultwidget.hpp \
        qtwidgets.hpp \
        sequencesform.hpp \
        sequencesmodel.hpp \
        sequencewidget.hpp \
        standarditemhelper.hpp \
        standardmodel.hpp \
        treeitem.hpp \
    processmethodview.hpp \
    centroidmethodmodel.hpp \
    isotopemethodmodel.hpp \
    chemicalformula.hpp \
    elementalcompmodel.hpp \
    mscalibratemodel.hpp \
    molwidget.hpp \
    isotopeform.hpp \
    isotopedelegate.hpp \
    qctable.hpp

FORMS += \
    centroidform.ui \
    elementalcompositionform.ui \
    logwidget.ui \
    mscalibrationform.ui \
    peakresulttable.ui \
    sequencesform.ui \
    sequencewidget.ui \
    isotopeform.ui

RESOURCES += \
    resources.qrc

#OTHER_FILES += \
#    qml/ProcessMethodEditor.qml \
#    qml/content/BusyIndicator.qml \
#    qml/content/CategoryDelegate.qml \
#    qml/content/ScrollBar.qml \
#    qml/content/EditCentroidMethod.qml \
#    qml/content/EditElementalCompMethod.qml \
#    qml/content/EditIntegrationMethod.qml \
#    qml/content/EditIsotopeMethod.qml \
#    qml/content/EditLockMassMethod.qml \
#    qml/content/EditMSCalibMethod.qml \
#    qml/content/EditPeakIDTable.qml \
#    qml/content/EditReportMethod.qml \
#    qml/content/EditTargetMethod.qml \
#    qml/content/ScanType.qml \
#    qml/content/CaptionText.qml \
#    qml/content/TextInputBox.qml \
#    qml/content/ScanTypeDetails.qml \
#    qml/content/MethodEditDelegate.qml \
#    qml/content/TitleText.qml

DEPENDPATH += qml qml/content

folder_01.source = qml
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01
IDE_QML_PATH = $$IDE_DATA_PATH/qtwidgets
include(../../qtplatz_qml.pri)
qtcAddDeployment()




