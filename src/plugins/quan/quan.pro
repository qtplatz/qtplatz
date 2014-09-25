DEFINES += QUAN_LIBRARY
PROVIDER = MS-Cheminformatics
QT += core svg printsupport

include(../../qtplatzplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)
INCLUDEPATH *= $$OUT_PWD/../../libs

# Quan files

SOURCES += quanplugin.cpp \
    mainwindow.cpp \
    quanmode.cpp \
    quandocument.cpp \
    panelswidget.cpp \
    paneldata.cpp \
    doubletabwidget.cpp \
    datasequencewidget.cpp \
    datasequencetree.cpp \
    compoundswidget.cpp \
    compoundstable.cpp \
    quanconfigform.cpp \
    quanconfigwidget.cpp \
    quandatawriter.cpp \
    quansampleprocessor.cpp \
    processmethodwidget.cpp \
    quanreportwidget.cpp \
    quanquerywidget.cpp \
    quanprocessor.cpp \
    quanresulttable.cpp \
    quanqueryform.cpp \
    quanconnection.cpp \
    quanquery.cpp \
    quanplotwidget.cpp \
    quanresultwnd.cpp \
    quanresultwidget.cpp \
    quancmpdwidget.cpp \
    quanplotdata.cpp \
    quanpublisher.cpp \
    quansvgplot.cpp \
    quanplot.cpp \
    quanfactory.cpp \
    quaneditor.cpp

HEADERS += quanplugin.hpp \
        quan_global.hpp \
        quanconstants.hpp \
        mainwindow.hpp \
        quanmode.hpp \
        quandocument.hpp \
        panelswidget.hpp \
        paneldata.hpp \
        doubletabwidget.hpp \
        datasequencewidget.hpp \
        datasequencetree.hpp \
        compoundswidget.hpp \
        compoundstable.hpp \
        quanconfigform.hpp \
        quanconfigwidget.hpp \
        quandatawriter.hpp \
        quansampleprocessor.hpp \
        processmethodwidget.hpp \
        quanreportwidget.hpp \
        quanquerywidget.hpp \
        quanprocessor.hpp \
        quanresulttable.hpp \
        quanqueryform.hpp \
        quanconnection.hpp \
        quanquery.hpp \
        quanplotwidget.hpp \
        quanresultwidget.hpp \
        quanresultwnd.hpp \
        quancmpdwidget.hpp \
        quanplotdata.hpp \
        quanpublisher.hpp \
        quanprogress.hpp \
        quansvgplot.hpp \
        quanplot.hpp \
        quanfactory.hpp \
        quaneditor.hpp

# Qt Creator linking
LIBS += -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adprot) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adplot) \
        -l$$qtLibraryTarget(adwidgets) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adpublisher) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser)

!win32 {
  LIBS += -lboost_system \
          -lboost_filesystem \
          -lboost_iostreams \
          -lboost_date_time \
          -lboost_iostreams \
          -lbz2
}

linux-*: LIBS += -lqwt -ldl
macx: QMAKE_LFLAGS+=-Wl,-search_paths_first

RESOURCES += \
    quan.qrc

FORMS += \
    doubletabwidget.ui \
    quanconfigform.ui \
    quanqueryform.ui
