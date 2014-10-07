include($$replace(_PRO_FILE_PWD_, ([^/]+$), \\1/\\1_dependencies.pri))
TARGET = $$QTC_PLUGIN_NAME

# for substitution in the .pluginspec
dependencyList = "<dependencyList>"
for(dep, QTC_PLUGIN_DEPENDS) {
    include($$PWD/plugins/$$dep/$${dep}_dependencies.pri)
    dependencyList += "        <dependency name=\"$$QTC_PLUGIN_NAME\" version=\"$$QTPLATZ_VERSION\"/>"
}
for(dep, QTC_PLUGIN_RECOMMENDS) {
    include($$PWD/plugins/$$dep/$${dep}_dependencies.pri)
    dependencyList += "        <dependency name=\"$$QTC_PLUGIN_NAME\" version=\"$$QTPLATZ_VERSION\" type=\"optional\"/>"
}
dependencyList += "    </dependencyList>"
dependencyList = $$join(dependencyList, $$escape_expand(\\n))

include(../qtplatz.pri)
include(config.pri)

# use gui precompiled header for plugins by default
# isEmpty(PRECOMPILED_HEADER):PRECOMPILED_HEADER = $$PWD/shared/qtplatz_gui_pch.h

isEmpty(PROVIDER) {
    PROVIDER = QtProject
} else {
    LIBS += -L$$IDE_PLUGIN_PATH/QtProject
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
    error("qtplatzplugin.pri: You must provide a TARGET")
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

PLUGINSPEC = $${_PRO_FILE_PWD_}/$${TARGET}.pluginspec

PLUGINSPEC_IN = $${PLUGINSPEC}.in
exists($$PLUGINSPEC_IN) {
    OTHER_FILES += $$PLUGINSPEC_IN
    QMAKE_SUBSTITUTES += $$PLUGINSPEC_IN
    PLUGINSPEC = $$OUT_PWD/$${TARGET}.pluginspec
    copy2build.output = $$DESTDIR/${QMAKE_FUNC_FILE_IN_stripOutDir}
} else {
    # need to support that for external plugins
    OTHER_FILES += $$PLUGINSPEC
    copy2build.output = $$DESTDIR/${QMAKE_FUNC_FILE_IN_stripSrcDir}
}

copy2build.input += PLUGINSPEC
isEmpty(vcproj):copy2build.variable_out = PRE_TARGETDEPS
copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
copy2build.name = COPY ${QMAKE_FILE_IN}
copy2build.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copy2build

# CONFIGFILE
exists( $${_PRO_FILE_PWD_}/$${TARGET}.config ) {
  CONFIGFILE = $${_PRO_FILE_PWD_}/$${TARGET}.config
  OTHER_FILES += $$CONFIGFILE
  copy2file.output = $$DESTDIR/$${TARGET}.config
  copy2file.input += CONFIGFILE
  isEmpty(vcproj):copy2file.variable_out = PRE_TARGETDEPS
  copy2file.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
  copy2file.name = COPY2FILE ${QMAKE_FILE_IN}
  copy2file.CONFIG += no_link
  QMAKE_EXTRA_COMPILERS += copy2file
}

greaterThan(QT_MAJOR_VERSION, 4) {
#   Create a Json file containing the plugin information required by
#   Qt 5's plugin system by running a XSLT sheet on the
#   pluginspec file before moc runs.
   
   XMLPATTERNS = $$(XMLPATTERNS)
#   message("XMLPATTERNS=" $${XMLPATTERNS})
   isEmpty( XMLPATTERNS ): XMLPATTERNS = $$targetPath($$[QT_INSTALL_BINS]/xmlpatterns)

    pluginspec2json.name = Create Qt 5 plugin json file
    pluginspec2json.input = PLUGINSPEC
    pluginspec2json.variable_out = GENERATED_FILES
    pluginspec2json.output = $${TARGET}.json
    pluginspec2json.commands = $$XMLPATTERNS -no-format -output $$pluginspec2json.output $$PWD/pluginjsonmetadata.xsl $$PLUGINSPEC
    pluginspec2json.CONFIG += no_link
    moc_header.depends += $$pluginspec2json.output
    QMAKE_EXTRA_COMPILERS += pluginspec2json
}

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
INCLUDEPATH *= $$PWD/libs $$PWD/adplugins

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

