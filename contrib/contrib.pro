TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)
include(../src/config.pri)

SUBDIRS += bruker

ExampleTOF: SUBDIRS += example-tof

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
