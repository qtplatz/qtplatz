#
cmake_minimum_required (VERSION 2.8.11)

if ( "${QTPLATZ_SOURCE_DIR}" STREQUAL "" )
  set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../.. )
  include( ${QTPLATZ_SOURCE_DIR}/qtplatz.cmake )
endif()

if (WIN32)

  add_subdirectory( wix )

else()

  include( qtplatz-cpack-config )

  set( CPACK_INSTALL_CMAKE_PROJECTS )
  add_subdirectory( ace+tao )
  list( APPEND CPACK_INSTALL_CMAKE_PROJECTS ${CMAKE_CURRENT_BINARY_DIR}/ace+tao;ace+tao;Runtime;/)
  
  add_subdirectory( boost )
  list( APPEND CPACK_INSTALL_CMAKE_PROJECTS ${CMAKE_CURRENT_BINARY_DIR}/boost;boost;Runtime;/)

endif()

