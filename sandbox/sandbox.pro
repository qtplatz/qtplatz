TEMPLATE = subdirs

SUBDIRS = meditor \
    qdirmodel \
    searchbox \
    signalsampling \
    molviewer \
    combination \
    polynomial_expansion

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
