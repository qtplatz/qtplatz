TEMPLATE = subdirs

SUBDIRS = sampling \
    qdirmodel \
    searchbox

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
