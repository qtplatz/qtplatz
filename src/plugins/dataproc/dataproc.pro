#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

QT       += xml

TARGET = dataproc
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../adiplugin.pri)
include(dataproc_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += ANALYSIS_LIBRARY

SOURCES += \
    dataprocplugin.cpp \
    dataprocmode.cpp \
    dataprocmanager.cpp \
    msprocessingwnd.cpp \
    elementalcompwnd.cpp \
    mscalibrationwnd.cpp \
    chromatogramwnd.cpp \
    dataset.cpp \
    dataprocessor.cpp \
    dataprocessorfactory.cpp \
    dataprocwidget.cpp \
    navigationwidgetfactory.cpp \
    navigationwidget.cpp \
    sessionmanager.cpp \
    navigationdelegate.cpp

HEADERS += \
    dataproc_global.h \
    dataprocplugin.h \
    dataprocmode.h \
    dataprocmanager.h \
    msprocessingwnd.h \
    elementalcompwnd.h \
    mscalibrationwnd.h \
    chromatogramwnd.h \
    dataset.h \
    dataprocessor.h \
    dataprocessorfactory.h \
    constants.h \
    dataprocwidget.h \
    navigationwidgetfactory.h \
    navigationwidget.h \
    sessionmanager.h \
    navigationdelegate.h

OTHER_FILES += dataproc.pluginspec \
    dataproc_dependencies.pri \
    application-data-mimetype.xml

RESOURCES += \
    dataproc.qrc
