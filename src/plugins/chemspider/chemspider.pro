TARGET = ChemSpider
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../boost.pri)

DEFINES += CHEMSPIDER_LIBRARY

SOURCES +=  chemspiderplugin.cpp \
            chemspidermode.cpp \
            chemspidermanager.cpp

HEADERS += chemspiderplugin.hpp\
        chemspider_global.hpp\
        chemspiderconstants.hpp \
        chemspidermode.hpp \
        chemspidermanager.hpp

OTHER_FILES = ChemSpider.pluginspec \
    ChemSpider.config.xml

LIBS += -L$$IDE_PLUGIN_PATH/Nokia -L$$IDE_LIBRARY_PATH

RESOURCES += \
    chemspider.qrc

FORMS += \
    chemspidermode.ui

