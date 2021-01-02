if ( ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL "19.27" )
  set( CRTDIR "$ENV{VCTOOLSREDISTDIR}/x64/Microsoft.VC142.CRT" )
elseif ( ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL "19.11" )
  set( CRTDIR "$ENV{VCTOOLSREDISTDIR}/x64/Microsoft.VC141.CRT" )
elseif( ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL "19.00" )
  set( CRTDIR "$ENV{VCTOOLSREDISTDIR}/x64/Microsoft.VC140.CRT" )
endif()

message( STATUS "###################################################################" )
message( STATUS "## CMAKE_CXX_COMPILER_VERSION: " ${CMAKE_CXX_COMPILER_VERSION} )
message( STATUS "## CRTDIR: " ${CRTDIR} )

file( GLOB files "${CRTDIR}/*.dll" )
foreach( file ${files} )
  file( TO_CMAKE_PATH ${file} cmake_file )
  message( STATUS "## ${cmake_file} --> ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY}" )
  install ( PROGRAMS "${cmake_file}" DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT applications )
endforeach()

message( STATUS "###################################################################" )

set ( REDIST "$ENV{VCTOOLSREDISTDIR}vc_redist.x64.exe" )
message( STATUS ${REDIST} )

if( EXISTS ${REDIST} )
  message( STATUS "VC Redistribution installer found: " ${REDIST} )
  file( TO_CMAKE_PATH ${REDIST} cmake_REDIST )
  install( PROGRAMS ${cmake_REDIST} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT applications )
else()
  message( FATAL "VC Redist installer NOT found")
endif()

message( STATUS "###################################################################" )
