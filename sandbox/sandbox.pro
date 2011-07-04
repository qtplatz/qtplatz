TEMPLATE = subdirs

SUBDIRS = meditor \
    qdirmodel \
    searchbox \
    signalsampling

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
