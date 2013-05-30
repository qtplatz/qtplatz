include($$replace(_PRO_FILE_PWD_, ([^/]+$), \\1/\\1_dependencies.pri))

include(../qtplatz.pri)
include( config.pri )

#win32 {
#    DLLDESTDIR = $$IDE_APP_PATH
#}

DESTDIR = $$IDE_LIBRARY_PATH

include(rpath.pri)

TARGET = $$qtLibraryTarget($$QTC_LIB_NAME)
TEMPLATE = lib
CONFIG += staticlib

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

!macx {
    win32 {
        target.path = /lib
        target.files = $$DESTDIR/$${TARGET}.lib
    } else {
        target.path = /$$IDE_LIBRARY_BASENAME/qtplatz
    }
    INSTALLS += target
}
