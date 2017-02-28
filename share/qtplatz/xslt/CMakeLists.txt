project( xslt )

set( XSLFILES
  quan-html.xsl
  quan-infusion-html.xsl
  quan-xhtml.xsl
  process-method-html.xsl )

file( GLOB files ${CMAKE_CURRENT_SOURCE_DIR}/*.xsl )

if ( APPLE )
  file( COPY ${files} DESTINATION ${CMAKE_BINARY_DIR}/bin/qtplatz.app/Contents/Resources/xslt )
else()
  file( COPY ${files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
endif()

install( FILES ${XSLFILES} DESTINATION share/qtplatz/xslt COMPONENT applications )

