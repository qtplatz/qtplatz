include(../../qtplatz.pri)
include(../shared/qtsingleapplication/qtsingleapplication.pri)
TEMPLATE = app
TARGET = $$IDE_APP_TARGET
DESTDIR = $$IDE_APP_PATH

SOURCES += main.cpp
# Qt4 is no longer supported due to using new signal/slot syntax
QT += widgets

include(../config.pri)
include(../rpath.pri)
include(../version.pri)

LIBS += -L$$IDE_LIBRARY_PATH

win32 {
    CONFIG(debug, debug|release):LIBS *= -lExtensionSystemd -lAggregationd
    else:LIBS *= -lExtensionSystem -lAggregation

    RC_FILE = qtplatz.rc
    target.path = /bin
    INSTALLS += target
} else:macx {
    CONFIG(debug, debug|release):LIBS *= -lExtensionSystem_debug -lAggregation_debug
    else:LIBS *= -lExtensionSystem -lAggregation
    LIBS += -framework CoreFoundation
    ICON = qtPlatz.icns
    QMAKE_INFO_PLIST = Info.plist
    FILETYPES.files = profile.icns prifile.icns
    FILETYPES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILETYPES
} else {
    LIBS *= -lExtensionSystem -lAggregation

    target.path  = /bin
    INSTALLS    += target
}

OTHER_FILES += qtplatz.rc

