
QT += core svg printsupport

DEFINES += DATAPROC_LIBRARY

include(../../qwt.pri)
include(../../boost.pri)

INCLUDEPATH *= $$OUT_PWD/../../libs
INCLUDEPATH *= $(QWT)/include

SOURCES += aboutdlg.cpp \
           dataprocplugin.cpp \
           mode.cpp \
           mainwindow.cpp \
           dataproc_document.cpp \
           navigationdelegate.cpp \
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
           elementalcompwnd.cpp \
           isequenceimpl.cpp \
           isnapshothandlerimpl.cpp \
           mscalibrationwnd.cpp \
           mscalibspectrawnd.cpp \
           msprocessingwnd.cpp \
           msspectrawnd.cpp \
           sessionmanager.cpp \
           mspropertyform.cpp \
           dataprocessworker.cpp \
           dialogspectrometerchoice.cpp \
           filepropertywidget.cpp \
           mspeakswnd.cpp \
           spectrogramwnd.cpp \
           ipeptidehandlerimpl.cpp

HEADERS += aboutdlg.hpp \
           dataproc_global.hpp \
           mimetypehelper.hpp \
           selchanged.hpp \
           qtwidgets_name.hpp \
           dataprocplugin.hpp \
           dataprocconstants.hpp \
           mode.hpp \
           mainwindow.hpp \
           dataproc_document.hpp \
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

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget(adplot) \
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

LIBS += -l$$qtLibraryTarget( adportable )
LIBS += -l$$qtLibraryTarget( xmlparser )

!win32 {
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem \
          -lboost_serialization
}

linux-*: LIBS += -lqwt -ldl -lboost_thread -lrt

PROVIDER = MS-Cheminformatics
include(../../qtplatzplugin.pri)

MIMEFILE += dataproc-mimetype.xml
xcopy2file.output = $$DESTDIR/$${QMAKE_FUNC_FILE_IN_stripSrcDir}
xcopy2file.input += MIMEFILE
isEmpty(vcproj):xcopy2file.variable_out = PRE_TARGETDEPS
xcopy2file.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
xcopy2file.name = XCOPY2FILE ${QMAKE_FILE_IN}
xcopy2file.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += xcopy2file

RESOURCES += \
    dataproc.qrc

FORMS += \
    mspropertyform.ui \
    dialogspectrometerchoice.ui

