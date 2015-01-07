TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)
include(../src/config.pri)

contains( QTPLATZ_CONFIG, ExampleTOF ) {
  SUBDIRS += example-tof
}

contains( QTPLATZ_CONFIG, Galactic ) {
  SUBDIRS += galactic
}

contains( QTPLATZ_CONFIG, Bruker ) {
  SUBDIRS += bruker
}

contains( QTPLATZ_CONFIG, Agilent ) {
  win32: SUBDIRS += agilent
}

contains( QTPLATZ_CONFIG, Shrader ) {
  win32: SUBDIRS += shrader
}

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
