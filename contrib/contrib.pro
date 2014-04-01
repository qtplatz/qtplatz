TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)
include(../src/config.pri)

SUBDIRS += bruker \
           galactic

win32: SUBDIRS += agilent

contains( QTPLATZ_CONFIG, ExampleTOF ) {
  SUBDIRS += example-tof
}

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
