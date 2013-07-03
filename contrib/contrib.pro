TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)

SUBDIRS += \
    bruker \
    example-tof

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
