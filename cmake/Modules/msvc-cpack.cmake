
if ( ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL "19.11" )
  set( CRTDIR "$ENV{VCTOOLSREDISTDIR}/x64/Microsoft.VC141.CRT" )
elseif( ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL "19.00" )
  set( CRTDIR "$ENV{VCTOOLSREDISTDIR}/x64/Microsoft.VC140.CRT" )
endif()

#message( STATUS "###################### " ${CRTDIR} )
file( GLOB files "${CRTDIR}/*.dll" )
foreach( file ${files} )
  message( STATUS "## " ${file} " --> " ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} )
  install ( PROGRAMS "${file}" DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT applications )
endforeach()
