TEMPLATE = subdirs

SUBDIRS = meditor \
    qdirmodel \
    searchbox \
    signalsampling \
    molviewer \
    combination \
    polynomial_expansion \
    boost_ssl_server \
    boost_ssl_client \
    chemspider \
    svgviewer

unix:!macx:!isEmpty(copydata):SUBDIRS += bin
