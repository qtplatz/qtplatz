QT += core svg printsupport
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

PROVIDER = MS-Cheminformatics

include(../../qtplatzplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)
INCLUDEPATH *= $$OUT_PWD/../../libs
DEFINES += PEPTIDE_LIBRARY

LIBS += -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adprot) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adwplot) \
        -l$$qtLibraryTarget(qtwrapper)

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

# peptide files

SOURCES += peptideplugin.cpp \
        mainwindow.cpp \
        peptidemode.cpp \
        proteintable.cpp \
        proteinwnd.cpp \
        digestedpeptidetable.cpp

HEADERS += peptideplugin.hpp \
        peptide_global.hpp \
        peptideconstants.hpp \
        mainwindow.hpp \
        peptidemode.hpp \
        proteintable.hpp \
        proteinwnd.hpp \
        digestedpeptidetable.hpp

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)

PROVIDER = MS-Cheminformatics

RESOURCES += \
    peptide.qrc

FORMS +=


