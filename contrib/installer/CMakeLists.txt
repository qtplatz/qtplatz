#
cmake_minimum_required (VERSION 2.8.11)

if ( "${QTPLATZ_SOURCE_DIR}" STREQUAL "" )
  set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../.. )
endif()

include( ${QTPLATZ_SOURCE_DIR}/qtplatz.cmake )

if (WIN32)
  add_subdirectory( wix )
endif()

if (CMAKE_SYSTEM_NAME MATCHES Linux )
  add_subdirectory( boost )
endif()
