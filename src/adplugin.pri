include($$replace(_PRO_FILE_PWD_, ([^/]+$), \\1/\\1_dependencies.pri))
TARGET = $$ADPLUGIN_NAME

# for substitution in the .pluginspec
dependencyList = "<dependencyList>"
for(dep, ADPLUGIN_DEPENDS) {
    include($$PWD/plugins/$$dep/$${dep}_dependencies.pri)
    dependencyList += "        <dependency name=\"$$QTC_PLUGIN_NAME\" version=\"$$QTPLATZ_VERSION\"/>"
}
dependencyList += "    </dependencyList>"
dependencyList = $$join(dependencyList, $$escape_expand(\\n))

include(../qtplatz.pri)

# use gui precompiled header for plugins by default
# isEmpty(PRECOMPILED_HEADER):PRECOMPILED_HEADER = $$PWD/shared/qtplatz_gui_pch.h

isEmpty(PROVIDER) {
    PROVIDER = MS-Cheminformatics
}

isEmpty(USE_USER_DESTDIR) {
    DESTDIR = $$IDE_PLUGIN_PATH/$$PROVIDER
} else {
    win32 {
        DESTDIRAPPNAME = "qtplatz"
        DESTDIRBASE = "$$(LOCALAPPDATA)"
        isEmpty(DESTDIRBASE):DESTDIRBASE="$$(USERPROFILE)\Local Settings\Application Data"
    } else:macx {
        DESTDIRAPPNAME = "Qt Platz"
        DESTDIRBASE = "$$(HOME)/Library/Application Support"
    } else:unix {
        DESTDIRAPPNAME = "qtplatz"
        DESTDIRBASE = "$$(XDG_DATA_HOME)"
        isEmpty(DESTDIRBASE):DESTDIRBASE = "$$(HOME)/.local/share/data"
        else:DESTDIRBASE = "$$DESTDIRBASE/data"
    }
    DESTDIR = "$$DESTDIRBASE/QtProject/$$DESTDIRAPPNAME/plugins/$$QTPLATZ_VERSION/$$PROVIDER"
}
LIBS += -L$$DESTDIR

# copy the plugin spec
isEmpty(TARGET) {
    error("adplugin.pri: You must provide a TARGET")
}

isEqual(QT_MAJOR_VERSION, 5) {

defineReplace(stripOutDir) {
    return($$relative_path($$1, $$OUT_PWD))
}

} else { # qt5

defineReplace(stripOutDir) {
    1 ~= s|^$$re_escape($$OUT_PWD/)||$$i_flag
    return($$1)
}

} # qt5

PLUGINSPEC = $$_PRO_FILE_PWD_/$${TARGET}.adplugin
OTHER_FILES += $$PLUGINSPEC
copy2build.output = $$DESTDIR/${QMAKE_FUNC_FILE_IN_stripSrcDir}

copy2build.input = PLUGINSPEC
isEmpty(vcproj):copy2build.variable_out = PRE_TARGETDEPS
copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
copy2build.name = COPY ${QMAKE_FILE_IN}
copy2build.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copy2build

macx {
    !isEmpty(TIGER_COMPAT_MODE) {
        QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../PlugIns/$${PROVIDER}/
    } else {
        QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/PlugIns/$${PROVIDER}/
        QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../../,-rpath,@executable_path/../
    }
} else:linux-* {
    #do the rpath by hand since it's not possible to use ORIGIN in QMAKE_RPATHDIR
    QMAKE_RPATHDIR += \$\$ORIGIN
    QMAKE_RPATHDIR += \$\$ORIGIN/..
    QMAKE_RPATHDIR += \$\$ORIGIN/../..
    IDE_PLUGIN_RPATH = $$join(QMAKE_RPATHDIR, ":")
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$${IDE_PLUGIN_RPATH}\'
    QMAKE_RPATHDIR =
}

# put .pro file directory in INCLUDEPATH
CONFIG += include_source_dir

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

TEMPLATE = lib
CONFIG += plugin plugin_with_soname
linux*:QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

!macx {
    target.path = $$QTC_PREFIX/$$IDE_LIBRARY_BASENAME/qtplatz/plugins/$$PROVIDER
    pluginspec.files += $${TARGET}.pluginspec
    pluginspec.path = $$QTC_PREFIX/$$IDE_LIBRARY_BASENAME/qtplatz/plugins/$$PROVIDER
    INSTALLS += target pluginspec
}

TARGET = $$qtLibraryName($$TARGET)

