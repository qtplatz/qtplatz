QT += core svg printsupport
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

PROVIDER = MS-Cheminformatics

include(../../qtplatzplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)
INCLUDEPATH *= $$OUT_PWD/../../libs

DEFINES += BATCHPROC_LIBRARY

# batchproc files

SOURCES += batchprocplugin.cpp \
    mainwindow.cpp \
    batchmode.cpp \
    droptargetform.cpp \
    dropwidget.cpp

HEADERS += batchprocplugin.hpp \
        batchproc_global.hpp \
        batchprocconstants.hpp \
    mainwindow.hpp \
    batchmode.hpp \
    droptargetform.hpp \
    dropwidget.hpp


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

