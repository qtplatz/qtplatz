#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

QT += core svg printsupport
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

PROVIDER = MS-Cheminformatics

include(../../qtplatzplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget(adwplot) \
        -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(adwidgets) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(chromatogr) \
        -l$$qtLibraryTarget(adprot) \
        -l$$qtLibraryTarget(adextension)

!win32 {
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem \
          -lboost_serialization
}

LIBS += -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( xmlparser )

linux-*: LIBS += -lqwt -ldl

INCLUDEPATH *= $$OUT_PWD/../../libs
INCLUDEPATH *= $(QWT)/include
DEFINES += ANALYSIS_LIBRARY

SOURCES += \
    actionmanager.cpp \
    assign_masses.cpp \
    assign_peaks.cpp \
    chromatogramwnd.cpp \
    mass_calibrator.cpp \
    dataproceditor.cpp \
    dataprocessor.cpp \
    dataprocessorfactory.cpp \
    dataprochandler.cpp \
    dataprocplugin.cpp \
    elementalcompwnd.cpp \
    ifileimpl.cpp \
    isequenceimpl.cpp \
    isnapshothandlerimpl.cpp \
    mscalibrationwnd.cpp \
    mscalibspectrawnd.cpp \
    msprocessingwnd.cpp \
    navigationdelegate.cpp \
    navigationwidget.cpp \
    navigationwidgetfactory.cpp \
    sessionmanager.cpp \
    mainwindow.cpp \
    mode.cpp \
    editorfactory.cpp \
    mspropertyform.cpp \
    dataprocessworker.cpp \
    dialogspectrometerchoice.cpp \
    filepropertywidget.cpp \
    mspeakswnd.cpp \
    spectrogramwnd.cpp \
    ipeptidehandlerimpl.cpp

HEADERS += \
    dataproc_global.h \
    actionmanager.hpp \
    chromatogramwnd.hpp \
    dataproceditor.hpp \
    dataprocessor.hpp \
    dataprocessorfactory.hpp \
    dataprochandler.hpp \
    dataprocplugin.hpp \
    elementalcompwnd.hpp \
    ifileimpl.hpp \
    isequenceimpl.hpp \
    isnapshothandlerimpl.hpp \
    mscalibrationwnd.hpp \
    mscalibspectrawnd.hpp \
    msprocessingwnd.hpp \
    navigationdelegate.hpp \
    navigationwidget.hpp \
    navigationwidgetfactory.hpp \
    sessionmanager.hpp \
    mainwindow.hpp \
    mode.hpp \
    editorfactory.hpp \
    assign_masses.hpp \
    mass_calibrator.hpp \
    assign_peaks.hpp \
    mspropertyform.hpp \
    dataprocessworker.hpp \
    dialogspectrometerchoice.hpp \
    filepropertywidget.hpp \
    mspeakswnd.hpp \
    spectrogramwnd.hpp \
    ipeptidehandlerimpl.hpp

OTHER_FILES += \
    dataproc.pluginspec \
    dataproc-mimetype.xml \
    dataproc.config.xml

PLUGINSPECS += dataproc-mimetype.xml

RESOURCES += \
    dataproc.qrc

FORMS += \
    mspropertyform.ui \
    dialogspectrometerchoice.ui
