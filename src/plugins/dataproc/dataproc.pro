#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

# QT       += xml

TARGET = dataproc
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)

include(../../libs/acewrapper/acewrapper_dependencies.pri)

LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)
LIBS += -l$$qtLibraryTarget(adwplot) -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adplugin) \
    -l$$qtLibraryTarget(adcontrols) -l$$qtLibraryTarget(adutils) -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adinterface) -l$$qtLibraryTarget(portfolio) -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(qwt) \
    -l$$qtLibraryTarget(chromatogr)

!win32 {
  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem
}

INCLUDEPATH *= $$OUT_PWD/../../libs
INCLUDEPATH *= $(QWT)/include
DEFINES += ANALYSIS_LIBRARY

SOURCES += \
    actionmanager.cpp \
    chromatogramwnd.cpp \
    dataproc.cpp \
    dataproceditor.cpp \
    dataprocessor.cpp \
    dataprocessorfactory.cpp \
    dataprochandler.cpp \
    dataprocmanager.cpp \
    dataprocmode.cpp \
    dataprocplugin.cpp \
    elementalcompwnd.cpp \
    ifileimpl.cpp \
    mscalibrationwnd.cpp \
    msprocessingwnd.cpp \
    navigationdelegate.cpp \
    navigationwidget.cpp \
    navigationwidgetfactory.cpp \
    sessionmanager.cpp \
    datafileobserver_i.cpp

HEADERS += \
    dataproc_global.h \
    actionmanager.hpp \
    chromatogramwnd.hpp \
    dataproceditor.hpp \
    dataprocessor.hpp \
    dataprocessorfactory.hpp \
    dataprochandler.hpp \
    dataprocmanager.hpp \
    dataprocmode.hpp \
    dataprocplugin.hpp \
    elementalcompwnd.hpp \
    ifileimpl.hpp \
    mscalibrationwnd.hpp \
    msprocessingwnd.hpp \
    navigationdelegate.hpp \
    navigationwidget.hpp \
    navigationwidgetfactory.hpp \
    sessionmanager.hpp \
    datafileobserver_i.hpp

OTHER_FILES += \
    dataproc.pluginspec \
    dataproc-mimetype.xml \
    dataproc.config.xml

PLUGINSPECS += dataproc-mimetype.xml

RESOURCES += \
    dataproc.qrc
