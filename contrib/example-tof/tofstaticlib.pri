#qmake file for static library
#library file output to qtplatz/lib/qtplatz

include( tof.pri )
IDE_BUILD_TREE = $${QTPLATZ_SOURCE_TREE}
include( $${QTPLATZ_SOURCE_TREE}/src/qtplatzstaticlib.pri )
