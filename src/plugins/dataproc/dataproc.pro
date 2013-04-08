#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

QT += core
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

TARGET = dataproc
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)

LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

LIBS += -l$$qtLibraryTarget(adwplot) -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) -l$$qtLibraryTarget(adutils) -l$$qtLibraryTarget(acewrapper) \
        -l$$qtLibraryTarget(adinterface) -l$$qtLibraryTarget(portfolio) -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(qwt) \
        -l$$qtLibraryTarget(chromatogr) \
        -l$$qtLibraryTarget(adextension)

!win32 {
  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem
}

# define BOOST_NO_CXX11_RVALUE_REFERENCES is a workaround on clang++ shipped 
# with Apple which does not provide std::move
#macx: QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++ # -std=c++11 -stdlib=libc++ -DBOOST_NO_CXX11_RVALUE_REFERENCES
macx: QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -std=c++11 -DBOOST_NO_CXX11_RVALUE_REFERENCES
CONFIG += c++11

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
    dataprocplugin.cpp \
    elementalcompwnd.cpp \
    ifileimpl.cpp \
    mscalibrationwnd.cpp \
    msprocessingwnd.cpp \
    navigationdelegate.cpp \
    navigationwidget.cpp \
    navigationwidgetfactory.cpp \
    sessionmanager.cpp \
    datafileobserver_i.cpp \
    mainwindow.cpp \
    mode.cpp \
    editorfactory.cpp \
    isequenceimpl.cpp

HEADERS += \
    dataproc_global.h \
    actionmanager.hpp \
    chromatogramwnd.hpp \
    dataproceditor.hpp \
    dataprocessor.hpp \
    dataprocessorfactory.hpp \
    dataprochandler.hpp \
    dataprocmanager.hpp \
    dataprocplugin.hpp \
    elementalcompwnd.hpp \
    ifileimpl.hpp \
    mscalibrationwnd.hpp \
    msprocessingwnd.hpp \
    navigationdelegate.hpp \
    navigationwidget.hpp \
    navigationwidgetfactory.hpp \
    sessionmanager.hpp \
    datafileobserver_i.hpp \
    mainwindow.hpp \
    mode.hpp \
    editorfactory.hpp \
    isequenceimpl.hpp

OTHER_FILES += \
    dataproc.pluginspec \
    dataproc-mimetype.xml \
    dataproc.config.xml

PLUGINSPECS += dataproc-mimetype.xml

RESOURCES += \
    dataproc.qrc
