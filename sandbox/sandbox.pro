TEMPLATE = subdirs

SUBDIRS = sampling \
    qdirmodel

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
