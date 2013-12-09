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
        moltableview.cpp \
        sdfile.cpp

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
        moltableview.hpp \
        sdfile.hpp

OTHER_FILES = chemistry.pluginspec

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget( adutils ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( qtwrapper ) \
        -l$$qtLibraryTarget( xmlparser )


!win32 {
    LIBS += -lboost_date_time -lboost_filesystem -lboost_system
}

LIBS += -lFileParsers -lGraphMol -lSmilesParse -lRDGeneral -lRDGeometryLib -lSubstructMatch -lDepictor


RESOURCES += \
    chemistry.qrc

FORMS += \
    sdfileview.ui \
    massdefectform.ui

#macx {
#  RDKIT_DLLS = libFileParsers libGraphMol libSmilesParse libRDGeneral libRDGeometryLib libSubstructMatch libDepictor

#  for (file, RDKIT_DLLS) FILES += $$replace( file, /, $$QMAKE_DIR_SEP)
#  dest = $$replace(IDE_APP_PATH, /, $$QMAKE_DIR_SEP)
#  for (file, FILES) {
#      QMAKE_POST_LINK +=$$quote($(COPY) $${file} $${dest}$$escape_expand(\\n\\t))
#      message( "file " $${QMAKE_POST_LINK} )
#  }
#}
