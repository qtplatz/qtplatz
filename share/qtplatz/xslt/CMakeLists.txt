project( xslt )

set( XSLFILES quan-html.xsl quan-infusion-html.xsl quan-xhtml.xsl )

file( GLOB files ${CMAKE_CURRENT_SOURCE_DIR}/*.xsl )
file( COPY ${files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

install( FILES ${XSLFILES} DESTINATION share/qtplatz/xslt COMPONENT applications )

