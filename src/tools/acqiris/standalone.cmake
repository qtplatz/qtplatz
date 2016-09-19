
set ( QTPLATZ_BUILD_DIR ${QTPLATZ_BUILD_DIR} CACHE STRING "destination directory" )

if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL "" )
  string( TOLOWER ${CMAKE_BUILD_TYPE} build_type )
else()
  set( build_type "release" )
endif()

find_file( qtplatz_config NAME "qtplatz-config.cmake" PATHS
  "${QTPLATZ_BUILD_DIR}"
  "${CMAKE_BINARY_DIR}/../qtplatz.netbeans"
  "${CMAKE_BINARY_DIR}/../qtplatz.${build_type}"
  )

if ( NOT qtplatz_config )
  message( FATAL_ERROR "qtplatz-config.cmake can not be found" )
endif()

include( ${qtplatz_config} )

