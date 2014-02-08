QT     += core gui svg
contains(QT_CONFIG, opengl) QT += opengl
TARGET = chemistry
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include(../../qtplatzplugin.pri)
include(../../boost.pri)
include(../../rdkit.pri)

DEFINES += CHEMISTRY_LIBRARY

# chemistry files

SOURCES += chemistryplugin.cpp \
        chemistrymode.cpp \
        moltabledelegate.cpp \
        chemfile.cpp \
        mainwindow.cpp \
        massdefectform.cpp \
        massdefectdelegate.cpp \
        massdefectmethod.cpp \
        moltableview.cpp

HEADERS += chemistryplugin.hpp\
        chemistry_global.hpp\
        moltabledelegate.hpp \
        chemistryconstants.hpp \
    	chemistrymode.hpp \
        chemfile.hpp \
        mainwindow.hpp \
        massdefectform.hpp \
        massdefectdelegate.hpp \
        massdefectmethod.hpp \
        moltableview.hpp

OTHER_FILES = chemistry.pluginspec

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget( adutils ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adchem ) \
        -l$$qtLibraryTarget( adwchem ) \
        -l$$qtLibraryTarget( adcontrols ) \
        -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( qtwrapper ) \
        -l$$qtLibraryTarget( adprot )

!win32 {
    LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}

LIBS += -lFileParsers -lGraphMol -lSmilesParse -lRDGeneral -lRDGeometryLib -lSubstructMatch -lDepictor -lDescriptors

RESOURCES += \
    chemistry.qrc

FORMS += \
    sdfileview.ui \
    massdefectform.ui

