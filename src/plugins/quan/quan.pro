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
    quandocument.cpp

HEADERS += quanplugin.hpp \
        quan_global.hpp \
        quanconstants.hpp \
    mainwindow.hpp \
    quanmode.hpp \
    quandocument.hpp

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
