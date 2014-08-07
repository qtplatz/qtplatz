
QT += core svg printsupport

DEFINES += DATAPROC_LIBRARY

include(../../qwt.pri)
include(../../boost.pri)

INCLUDEPATH *= $$OUT_PWD/../../libs
INCLUDEPATH *= $(QWT)/include

SOURCES += dataprocplugin.cpp \
           mode.cpp \
           mainwindow.cpp \
           document.cpp \
           navigationwidgetfactory.cpp \
           navigationwidget.cpp \
           dataprocessor.hpp \
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
           msspectrawnd.cpp \
           navigationdelegate.cpp \
           navigationwidget.cpp \
           navigationwidgetfactory.cpp \
           sessionmanager.cpp \
           editorfactory.cpp \
           mspropertyform.cpp \
           dataprocessworker.cpp \
           dialogspectrometerchoice.cpp \
           filepropertywidget.cpp \
           mspeakswnd.cpp \
           spectrogramwnd.cpp \
           ipeptidehandlerimpl.cpp \
           document.cpp

HEADERS += dataproc_global.hpp \
           mimetypehelper.hpp \
           selchanged.hpp \
           qtwidgets_name.hpp \
           dataprocplugin.hpp \
           dataprocconstants.hpp \
           mode.hpp \
           mainwindow.hpp \
           document.hpp \
           navigationwidgetfactory.hpp \
           navigationwidget.hpp \
           dataprocessor.hpp \
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
           msspectrawnd.hpp \
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
           ipeptidehandlerimpl.hpp \
           document.hpp

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

QTC_PLUGIN_NAME = Dataproc
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

PROVIDER = MS-Cheminformatics
include(../../qtplatzplugin.pri)

#include($$QTCREATOR_SOURCES/src/qtplatzplugin.pri)

PLUGINSPECS += dataproc-mimetype.xml

RESOURCES += \
    dataproc.qrc

FORMS += \
    mspropertyform.ui \
    dialogspectrometerchoice.ui

