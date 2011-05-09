#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

QT       += xml

TARGET = dataproc
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(dataproc_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

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
    sessionmanager.cpp

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
    sessionmanager.hpp

OTHER_FILES += dataproc.pluginspec \
    dataproc_dependencies.pri \
    application-data-mimetype.xml

RESOURCES += \
    dataproc.qrc
