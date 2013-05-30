QT     += core gui svg
contains(QT_CONFIG, opengl) QT += opengl
TARGET = chemistry
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include(../../qtplatzplugin.pri)
include(../../boost.pri)
include(../../openbabel.pri)
include(../../ace_tao.pri)

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
    svgitem.cpp \
    massdefectform.cpp \
    massdefectdelegate.cpp \
    massdefectmethod.cpp

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
        svgitem.hpp \
        massdefectform.hpp \
        massdefectdelegate.hpp \
        massdefectmethod.hpp

OTHER_FILES = chemistry.pluginspec

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget( adutils ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( qtwrapper ) \
        -l$$qtLibraryTarget( xmlparser ) \
	-l$$qtLibraryTarget( adchem )
LIBS += -lopenbabel-2

RESOURCES += \
    chemistry.qrc

FORMS += \
    sdfileview.ui \
    massdefectform.ui

win32 {
    for(file, OPENBABEL_DLLS ) FILES += $$replace(file, /, $$QMAKE_DIR_SEP)
    for(file, OPENBABEL_FILES) FILES += $$replace(file, /, $$QMAKE_DIR_SEP)
    dest = $$replace(IDE_APP_PATH, /, $$QMAKE_DIR_SEP)
    for(file, FILES) {
       QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${file} $${dest}$$escape_expand(\\n\\t))
    }
} else {
    for(file, OPENBABEL_DLLS ) FILES += $$replace(file, /, $$QMAKE_DIR_SEP)
    dest = $$replace(IDE_LIBRARY_PATH, /, $$QMAKE_DIR_SEP)
    for(file, FILES) {
       QMAKE_POST_LINK +=$$quote($(COPY) $${file} $${dest}$$escape_expand(\\n\\t))
    }
}
