
include(../qtplatz.pri)
include( config.pri )

isEmpty(PROVIDER) PROVIDER = MS-Cheminformatics

DESTDIR = $$IDE_PLUGIN_PATH/$$PROVIDER
LIBS += -L$$DESTDIR
INCLUDEPATH += $$IDE_SOURCE_TREE/src/plugins
DEPENDPATH += $$IDE_SOURCE_TREE/src/plugins

isEmpty(TARGET) {
    error("qtplutz_servant.pri: You must provide a TARGET")
}

TARGET = $$qtLibraryTarget($$TARGET)

include( rpath.pri )

# put .pro file directory in INCLUDEPATH
CONFIG += include_source_dir
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

TEMPLATE = lib
#CONFIG += shared dll
CONFIG += plugin plugin_with_soname

!macx {
    target.path = $$QTC_PREFIX/$$IDE_LIBRARY_BASENAME/qtplatz/plugins/$$PROVIDER
    pluginspec.path = $$QTC_PREFIX/$$IDE_LIBRARY_BASENAME/qtplatz/plugins/$$PROVIDER
    INSTALLS += target pluginspec
}
