QT     += core gui svg
contains(QT_CONFIG, opengl) QT += opengl
TARGET = chemistry
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../boost.pri)
include(../../openbabel.pri)

DEFINES += CHEMISTRY_LIBRARY

# chemistry files

SOURCES += chemistryplugin.cpp \
    chemistrymode.cpp \
    sdfileview.cpp \
    chemeditor.cpp \
    chemeditorfactory.cpp \
    chemfile.cpp \
    chemistrymainwindow.cpp \
    sdfilemodel.cpp \
    sdfiledelegate.cpp \
    svgitem.cpp

HEADERS += chemistryplugin.hpp\
        chemistry_global.hpp\
        chemistryconstants.hpp \
    	chemistrymode.hpp \
    	sdfileview.hpp \
    chemeditor.hpp \
    chemeditorfactory.hpp \
    chemfile.hpp \
    chemistrymainwindow.hpp \
    sdfilemodel.hpp \
    sdfiledelegate.hpp \
    svgitem.hpp

OTHER_FILES = chemistry.pluginspec

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
    for(file, OPENBABEL_DLLS ) FILES += $$replace(file, /, $$QMAKE_DIR_SEP)
    for(file, OPENBABEL_FILES) FILES += $$replace(file, /, $$QMAKE_DIR_SEP)
    dest = $$replace(IDE_APP_PATH, /, $$QMAKE_DIR_SEP)
    for(file, FILES) {
       QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${file} $${dest}$$escape_expand(\\n\\t))
    }
}
