TARGET = Chemistry
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../boost.pri)
include(../../openbabel.pri)

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

win32 {
    # for(file, OPENBABEL_DLLS)  FILE += $$replace(file, /, $$QMAKE_DIR_SEP)
    copy2build.input = OPENBABEL_DLLS
    copy2build.output = $$IDE_APP_PATH
    isEmpty(vcproj):copy2build.variable_out = PRE_TARGETDEPS
    copy2build.commands = $${QMAKE_COPY} \"${QMAKE_FILE_IN}\" \"${QMAKE_FILE_OUT}\"
    copy2build.name = COPY ${QMAKE_FILE_IN}
    copy2build.CONFIG += no_link no_clean
    QMAKE_EXTRA_COMPILERS += copy2build
}