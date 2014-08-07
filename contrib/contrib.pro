TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)
include(../src/config.pri)

QTPLATZ_CONFIG += Bruker
QTPLATZ_CONFIG += Galactic
QTPLATZ_CONFIG += Agilent

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

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
