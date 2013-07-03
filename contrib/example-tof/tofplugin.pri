#qmake file for QtPlatz plugin
#library file output to qtplatz/lib/qtplatz/plubins

include( tof.pri )
IDE_BUILD_TREE = $${QTPLATZ_SOURCE_TREE}
include( $${QTPLATZ_SOURCE_TREE}/src/qtplatzplugin.pri )
