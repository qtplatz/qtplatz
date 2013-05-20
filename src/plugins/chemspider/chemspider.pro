QT += webkit
TARGET = ChemSpider
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include( ../../qtplatz_plugin.pri )
include( ../../plugins/coreplugin/coreplugin.pri )
include( ../../boost.pri )
include( ../../ace_tao.pri )

DEFINES += CHEMSPIDER_LIBRARY

SOURCES +=  chemspiderplugin.cpp \
            chemspidermode.cpp \
            chemspidermanager.cpp \
    massspecform.cpp

HEADERS += chemspiderplugin.hpp\
        chemspider_global.hpp\
        chemspiderconstants.hpp \
        chemspidermode.hpp \
        chemspidermanager.hpp \
    massspecform.hpp

OTHER_FILES = ChemSpider.pluginspec \
    ChemSpider.config.xml

LIBS += -L$$IDE_PLUGIN_PATH/Nokia -L$$IDE_LIBRARY_PATH
LIBS += -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser) 

RESOURCES += \
    chemspider.qrc

FORMS += \
    chemspidermode.ui \
    massspecform.ui

!win32 {
  LIBS *= -lboost_serialization -lboost_date_time -lboost_filesystem -lboost_system
}
