#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

QT += core gui
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

TARGET = qtwidgets
TEMPLATE = lib
PROVIDER = MS-Cheminformatics

include(../../adplugin.pri)
include(../../boost.pri)

LIBS += -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adplugin) -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser)

!win32 {
  LIBS *= -lboost_filesystem -lboost_system
}

INCLUDEPATH *= $(ACE_ROOT) $(TAO_ROOT)

#workaround for 5.1.0 cause a fatal error on qitemdelegate.h can not find qabstractitemdelegate.h
greaterThan( QT_MAJOR_VERSION, 4 ): INCLUDEPATH += ${QTDIR}/include  

DEFINES += QTWIDGETS_LIBRARY

SOURCES += centroiddelegate.cpp \
        centroidmethodmodel.cpp \
        elementalcompositiondelegate.cpp \
        elementalcompmodel.cpp \
        elementalcompositionform.cpp \
        factory.cpp \
        isotopedelegate.cpp \
        isotopeform.cpp \
        isotopemethodmodel.cpp \
        logwidget.cpp \
        molwidget.cpp \
        peakmethoddelegate.cpp \
        peakresultwidget.cpp \
        peakmethodform.cpp \
        # processmethodview.cpp \
        qtwidgets.cpp \
        sequencesform.cpp \
        sequencesmodel.cpp \
        sequencewidget.cpp \
        standarditemhelper.cpp \
        standardmodel.cpp \
        treeitem.cpp \
        chemicalformula.cpp \ 
        tabledelegate.cpp \
        adductsdelegate.cpp \
        formulaedelegate.cpp \
    timeeventsdelegate.cpp

HEADERS += centroiddelegate.hpp \
        elementalcompositiondelegate.hpp \
        elementalcompositionform.hpp \
        factory.hpp \
        logwidget.hpp \
        peakresultwidget.hpp \
        qtwidgets.hpp \
        sequencesform.hpp \
        sequencesmodel.hpp \
        sequencewidget.hpp \
        standarditemhelper.hpp \
        standardmodel.hpp \
        treeitem.hpp \
        # processmethodview.hpp \
        centroidmethodmodel.hpp \
        isotopemethodmodel.hpp \
        chemicalformula.hpp \
        elementalcompmodel.hpp \
        molwidget.hpp \
        isotopeform.hpp \
        isotopedelegate.hpp \
        peakmethodform.hpp \
        peakmethoddelegate.hpp \
        tabledelegate.hpp \
        adductsdelegate.hpp \
        formulaedelegate.hpp \
    timeeventsdelegate.hpp

FORMS += \
    elementalcompositionform.ui \
    logwidget.ui \
    mscalibrationform.ui \
    peakresulttable.ui \
    sequencesform.ui \
    sequencewidget.ui \
    isotopeform.ui \
    peakmethodform.ui

RESOURCES +=

#DEPENDPATH += qml qml/content

#folder_01.source = qml
#folder_01.target = qml
#DEPLOYMENTFOLDERS = folder_01
#IDE_QML_PATH = $$IDE_DATA_PATH/qtwidgets
#include(../../qtplatz_qml.pri)
#qtcAddDeployment()
