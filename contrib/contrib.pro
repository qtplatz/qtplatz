TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)

SUBDIRS += \
    bruker

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
