
# CONFIG += debug

defineReplace(cleanPath) {
    win32:1 ~= s|\\\\|/|g
    contains(1, ^/.*):pfx = /
    else:pfx =
    segs = $$split(1, /)
    out =
    for(seg, segs) {
        equals(seg, ..):out = $$member(out, 0, -2)
        else:!equals(seg, .):out += $$seg
    }
    return($$join(out, /, $$pfx))
}

defineReplace(targetPath) {
    return($$replace(1, /, $$QMAKE_DIR_SEP))
}

# For use in custom compilers which just copy files
win32:i_flag = i
defineReplace(stripSrcDir) {
    win32 {
        !contains(1, ^.:.*):1 = $$OUT_PWD/$$1
    } else {
        !contains(1, ^/.*):1 = $$OUT_PWD/$$1
    }
    out = $$cleanPath($$1)
    out ~= s|^$$re_escape($$PWD/)||$$i_flag
    return($$out)
}

isEmpty(TEST):CONFIG(debug, debug|release) {
    !debug_and_release|build_pass {
        TEST = 1
    }
}

isEmpty(IDE_LIBRARY_BASENAME) {
    IDE_LIBRARY_BASENAME = lib
}

DEFINES += IDE_LIBRARY_BASENAME=\\\"$$IDE_LIBRARY_BASENAME\\\"

equals(TEST, 1) {
    QT +=testlib
    DEFINES += WITH_TESTS
}

IDE_SOURCE_TREE = $$PWD
QTPLATZ_SOURCE_TREE = $$IDE_SOURCE_TREE/../qtplatz

######## Provider #######
PROVIDER = MSI

isEmpty(IDE_BUILD_TREE) {
    sub_dir = $$_PRO_FILE_PWD_
    sub_dir ~= s,^$$re_escape($$PWD),,
    IDE_BUILD_TREE = $$cleanPath($$OUT_PWD)
    IDE_BUILD_TREE ~= s,$$re_escape($$sub_dir)$,,
}
IDE_APP_PATH = $$IDE_BUILD_TREE/bin
macx {
    IDE_APP_TARGET   = "qtplatz"
    IDE_APP_PATH     = $$QTPLATZ_SOURCE_TREE/bin
    IDE_LIBRARY_PATH = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/PlugIns
    IDE_STATICLIB_PATH = $$IDE_BUILD_TREE/lib
    IDE_PLUGIN_PATH  = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/PlugIns
    IDE_LIBEXEC_PATH = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/Resources
    IDE_DATA_PATH    = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/Resources
    IDE_DOC_PATH     = $$IDE_DATA_PATH/doc/MSI
    QTPLATZ_BUILD_TREE = $$QTPLATZ_SOURCE_TREE
    QTPLATZ_PLUGIN_PATH = $$IDE_PLUGIN_PATH
    contains(QT_CONFIG, ppc):CONFIG += ppc x86
    copydata = 1
} else {
    win32 {
        contains(TEMPLATE, vc.*)|contains(TEMPLATE_PREFIX, vc):vcproj = 1
        IDE_APP_TARGET   = qtplatz
    } else {
        IDE_APP_WRAPPER  = qtplatz
        IDE_APP_TARGET   = qtplatz.bin
    }
    IDE_LIBRARY_PATH =   $$IDE_BUILD_TREE/$$IDE_LIBRARY_BASENAME
    IDE_STATICLIB_PATH = $$IDE_BUILD_TREE/lib
#    IDE_PLUGIN_PATH  = $$IDE_LIBRARY_PATH/plugins
    IDE_LIBEXEC_PATH = $$IDE_APP_PATH # FIXME
    IDE_DATA_PATH    = $$IDE_BUILD_TREE/share/MSI
    IDE_DOC_PATH     = $$IDE_BUILD_TREE/share/doc/MSI
    QTPLATZ_BUILD_TREE   = $$QTPLATZ_SOURCE_TREE
    QTPLATZ_LIBRARY_PATH = $$IDE_BUILD_TREE/$$IDE_LIBRARY_BASENAME/$$IDE_APP_TARGET
    QTPLATZ_PLUGIN_PATH  = $$QTPLATZ_BUILD_TREE/lib/qtplatz/plugins
    !isEqual(IDE_SOURCE_TREE, $$IDE_BUILD_TREE):copydata = 1
}

## set the QTC_SOURCE environment variable to override the setting here
QTPLATZ_SOURCES = $$(QTC_SOURCE)
isEmpty(QTPLATZ_SOURCES):QTPLATZ_SOURCES=$$QTPLATZ_SOURCE_TREE

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=$$QTPLATZ_BUILD_TREE

INCLUDEPATH += \
    $$IDE_SOURCE_TREE/src/libs \
    $$QTPLATZ_SOURCE_TREE/src/libs

DEPENDPATH += \
    $$IDE_SOURCE_TREE/src/libs \
    $$QTPLATZ_SOURCE_TREE/src/libs

LIBS += -L$$IDE_LIBRARY_PATH

# DEFINES += QT_NO_CAST_FROM_ASCII
DEFINES += QT_NO_CAST_TO_ASCII
#DEFINES += QT_USE_FAST_OPERATOR_PLUS
#DEFINES += QT_USE_FAST_CONCATENATION

unix {
    CONFIG(debug, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/debug-shared
    CONFIG(release, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/release-shared

    CONFIG(debug, debug|release):MOC_DIR = $${OUT_PWD}/.moc/debug-shared
    CONFIG(release, debug|release):MOC_DIR = $${OUT_PWD}/.moc/release-shared

    RCC_DIR = $${OUT_PWD}/.rcc
    UI_DIR = $${OUT_PWD}/.uic
}

linux-g++-* {
    # Bail out on non-selfcontained libraries. Just a security measure
    # to prevent checking in code that does not compile on other platforms.
    QMAKE_LFLAGS += -Wl,--allow-shlib-undefined -Wl,--no-undefined
}

# Handle S60 support: default on Windows, conditionally built on other platforms.
win32:SUPPORT_QT_S60=1
else:SUPPORT_QT_S60 = $$(QTCREATOR_WITH_S60)
