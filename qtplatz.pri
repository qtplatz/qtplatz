isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE  = ../qt-creator
include(../qt-creator/qtcreator.pri)

#IDE_SOURCE_TREE is now ../qt-creator defined in qtcreator.pri
QTPLATZ_SOURCE_TREE = $$IDE_SOURCE_TREE

#message("qtplatz.pri:IDE_BUILD_TREE: " $$IDE_BUILD_TREE )
#message("qtplatz.pri:IDE_LIBRARY_PATH: " $$IDE_LIBRARY_PATH )

INCLUDEPATH += \
    $$IDE_SOURCE_TREE/src/libs \
    $$IDE_SOURCE_TREE/tools

DEPENDPATH += \
    $$IDE_SOURCE_TREE/src/libs \
    $$IDE_SOURCE_TREE/tools

LIBS += -L$$IDE_LIBRARY_PATH

