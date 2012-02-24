TEMPLATE = subdirs

SUBDIRS = meditor \
    qdirmodel \
    searchbox \
    signalsampling \
    molviewer \
    combination

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
