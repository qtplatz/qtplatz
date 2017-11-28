#
cmake_minimum_required (VERSION 2.8.11)

if ( "${QTPLATZ_SOURCE_DIR}" STREQUAL "" )
  set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../.. )
  include( ${QTPLATZ_SOURCE_DIR}/qtplatz.cmake )
endif()

if (WIN32)

  add_subdirectory( wix )
  add_subdirectory( opencv )

elseif( APPLE )

  add_subdirectory( mac )  

else()

  message(STATUS "On Linux(debian), independent .deb package for qt5, ace+tao and boost should be created" )

endif()



