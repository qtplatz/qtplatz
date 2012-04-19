TEMPLATE = subdirs
CONFIG	+= ordered

include(contrib.pri)

SUBDIRS += \
    bruker \
    mzxml

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
