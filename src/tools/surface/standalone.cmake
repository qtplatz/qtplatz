set ( QTPLATZ_BUILD_DIR ${QTPLATZ_BUILD_DIR} CACHE STRING "destination directory" )

if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL "" )
  string( TOLOWER ${CMAKE_BUILD_TYPE} build_type )
else()
  set( build_type "release" )
endif()

find_file( qtplatz_config NAME "qtplatz-config.cmake" PATHS
  "$ENV{HOME}/src/build-Darwin-i386/qtplatz.release"
  "$ENV{HOME}/src/build-Linux-x86_64/qtplatz.release"
  "${QTPLATZ_BUILD_DIR}" )

if ( qtplatz_config )
  include( ${qtplatz_config} )
else()
  message( FATAL_ERROR "### ERROR ##### " ${qtplatz_config} )
endif()

list( APPEND CMAKE_PREFIX_PATH "${QTDIR}/lib/cmake" )

include_directories( ${CMAKE_SOURCE_DIR}/../../libs )                      # qtplatz/src/libs

find_package( Boost 1.67 REQUIRED COMPONENTS system )
if ( Boost_FOUND )
  include_directories( ${Boost_INCLUDE_DIRS} )
endif()
set( CMAKE_CXX_STANDARD 17 )
