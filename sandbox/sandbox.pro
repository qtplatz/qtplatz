TEMPLATE = subdirs

SUBDIRS = meditor \
    qdirmodel \
    searchbox \
    signalsampling \
    molviewer

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
