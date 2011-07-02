TEMPLATE = subdirs

SUBDIRS = meditor \
    qdirmodel \
    searchbox

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
