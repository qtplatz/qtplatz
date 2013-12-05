QT += core svg printsupport
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

PROVIDER = MS-Cheminformatics

include(../../qtplatzplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)
INCLUDEPATH *= $$OUT_PWD/../../libs
DEFINES += BATCHPROC_LIBRARY


LIBS += -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adextension)

!win32 {
  LIBS += -lboost_system \
          -lboost_filesystem \
          -lboost_iostreams \
          -lboost_date_time \
          -lboost_iostreams \
          -lbz2
}
linux-*: LIBS += -ldl
macx: QMAKE_LFLAGS+=-Wl,-search_paths_first

# batchproc files

SOURCES += batchprocplugin.cpp \
    mainwindow.cpp \
    batchmode.cpp \
    droptargetform.cpp \
    dropwidget.cpp \
    batchprocdelegate.cpp \
    task.cpp \
    import.cpp \
    process.cpp \
    datainterpreter.cpp \
    massspectrometerfactory.cpp \
    massspectrometer.cpp

HEADERS += batchprocplugin.hpp \
        batchproc_global.hpp \
        batchprocconstants.hpp \
        mainwindow.hpp \
        batchmode.hpp \
        droptargetform.hpp \
        dropwidget.hpp \
        batchprocdelegate.hpp \
        task.hpp \
        import.hpp \
        importdata.hpp \
        process.hpp \
    datainterpreter.hpp \
    massspectrometerfactory.hpp \
    massspectrometer.hpp


## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=C:/Users/Toshi/src/qtplatz

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=C:/Users/Toshi/src/qtplatz

PROVIDER = MS-Cheminformatics

#include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

RESOURCES += \
    batchproc.qrc

FORMS += \
    droptargetform.ui

