include(../qtplatz.pri)
include( config.pri )

isEmpty(PROVIDER) PROVIDER = ScienceLiaison

DESTDIR = $$IDE_PLUGIN_PATH/$$PROVIDER
LIBS += -L$$DESTDIR
INCLUDEPATH += $$IDE_SOURCE_TREE/src/plugins
DEPENDPATH += $$IDE_SOURCE_TREE/src/plugins

# copy the plugin spec
isEmpty(TARGET) {
    error("qtplutz_servant.pri: You must provide a TARGET")
}

TARGET = $$qtLibraryTarget($$TARGET)

macx {
        QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../PlugIns/$${PROVIDER}/
} else:linux-* {
    #do the rpath by hand since it's not possible to use ORIGIN in QMAKE_RPATHDIR
    QMAKE_RPATHDIR += \$\$ORIGIN
    QMAKE_RPATHDIR += \$\$ORIGIN/..
    QMAKE_RPATHDIR += \$\$ORIGIN/../..
    IDE_PLUGIN_RPATH = $$join(QMAKE_RPATHDIR, ":")
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$${IDE_PLUGIN_RPATH}\'
    QMAKE_RPATHDIR =
}

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

CONFIG += plugin plugin_with_soname

!macx {
    target.path = /$$IDE_LIBRARY_BASENAME/qtplatz/plugins/$$PROVIDER
    pluginspec.files += $${TARGET}.pluginspec
    pluginspec.path = /$$IDE_LIBRARY_BASENAME/qtplatz/plugins/$$PROVIDER
    INSTALLS += target pluginspec
    !exists( target.path ) { mkdir( target.path ) }
}
