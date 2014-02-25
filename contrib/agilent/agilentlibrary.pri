#qmake file for QtPlatz plugin

include( agilent.pri )
IDE_BUILD_TREE = $${QTPLATZ_SOURCE_TREE}
include( $${QTPLATZ_SOURCE_TREE}/src/qtplatzlibrary.pri )
