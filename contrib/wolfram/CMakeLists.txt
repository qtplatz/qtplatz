cmake_minimum_required(VERSION 2.6.2)

message( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )

if ( "${CMAKE_MODULE_PATH}" STREQUAL "" )
  message( STATUS ".... loading standalone.cmake for Wolfram project" )
  include( "standalone.cmake" )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin )
endif()

message( STATUS "---> CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )
# set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../../cmake/Modules" ${CMAKE_MODULE_PATH})

find_package ( Wolfram )

message( STATUS "Wolfram FOUND:           " ${Wolfram_FOUND} )
message( STATUS "WSTP_INCLUDE_DIRS :      " "${WSTP_INCLUDE_DIRS}" )

if ( Wolfram_FOUND )

  if ( APPLE )
    set( WSTP_MODULE_DIR ${CMAKE_BINARY_DIR}/bin/qtplatz.app/Library/Wolfram )
  else()
    set( WSTP_MODULE_DIR ${CMAKE_BINARY_DIR}/Wolfram )
  endif()

  set( WSTP_DIST_PACKAGES "${CMAKE_INSTALL_PREFIX}/Library/Wolfram/WSTP" )

  #subdirs( addtwo formula )
  subdirs( addtwo )
endif()
