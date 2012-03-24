TARGET = ChemSpider
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../boost.pri)

DEFINES += CHEMSPIDER_LIBRARY

SOURCES += chemspiderplugin.cpp

HEADERS += chemspiderplugin.hpp\
        chemspider_global.hpp\
        chemspiderconstants.hpp

OTHER_FILES = ChemSpider.pluginspec

LIBS += -L$$IDE_PLUGIN_PATH/Nokia -L$$IDE_LIBRARY_PATH

