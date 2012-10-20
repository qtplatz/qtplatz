TARGET = Chemistry
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../boost.pri)

DEFINES += CHEMISTRY_LIBRARY

# Chemistry files

SOURCES += chemistryplugin.cpp \
    chemistrymode.cpp \
    chemistrymanager.cpp \
    sdfileview.cpp

HEADERS += chemistryplugin.hpp\
        chemistry_global.hpp\
        chemistryconstants.hpp \
    chemistrymode.hpp \
    chemistrymanager.hpp \
    sdfileview.hpp

OTHER_FILES = Chemistry.pluginspec

LIBS += -L$$IDE_PLUGIN_PATH/Nokia -L$$IDE_LIBRARY_PATH
LIBS += -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser) 

RESOURCES += \
    chemistry.qrc

FORMS += \
    sdfileview.ui

