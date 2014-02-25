TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)
include(../src/config.pri)

SUBDIRS += bruker \
           agilent

contains( QTPLATZ_CONFIG, ExampleTOF ) {
  SUBDIRS += example-tof
}

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
