DEFINES += QUAN_LIBRARY
PROVIDER = MS-Cheminformatics

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
    quanreportwidget.cpp

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
    quanreportwidget.hpp

# Qt Creator linking
LIBS += -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adprot) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adwplot) \
        -l$$qtLibraryTarget(adwidgets) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper)

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
    dataselectionform.ui \
    quanconfigform.ui
