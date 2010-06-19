# -------------------------------------------------
# Project created by QtCreator 2010-06-05T17:24:25
# -------------------------------------------------
QT += svg \
    xml \
    webkit

TARGET = dataanalysis
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../qtPlatzplugin.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)
DEFINES += DATAANALYSIS_LIBRARY
SOURCES += dataanalysisplugin.cpp \
    dataanalysiswindow.cpp \
    datafile.cpp \
    dataeditorfactory.cpp \
    dataeditor.cpp \
    sidebar.cpp \
    minisplitter.cpp \
    openeditorsview.cpp \
    dataanalysismanager.cpp \
    outputwindow.cpp
HEADERS += dataanalysisplugin.h \
    dataanalysiswindow.h \
    datafile.h \
    dataeditorfactory.h \
    dataeditor.h \
    sidebar.h \
    minisplitter.h \
    openeditorsview.h \
    dataanalysismanager.h \
    outputwindow.h
OTHER_FILES += dataanalysis.pluginspec

INCLUDEPATH += /usr/local/include \
                c:/Boost/include/boost-1_43

RESOURCES += \
    dataanalysis.qrc

FORMS += \
    treewidget.ui
