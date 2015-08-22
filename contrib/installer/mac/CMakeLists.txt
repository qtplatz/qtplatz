#
cmake_minimum_required(VERSION 2.8.11)
project( mac_installer )

include( qtplatz-version )

set( QTPLATZ_VERSION ${git_describe} )

set( QTPLATZ_SOURCE_DIR "${CMAKE_SOURCE_DIR}" )
set( QTPLATZ_BINARY_DIR "${CMAKE_BINARY_DIR}" )

configure_file(
  "${PROJECT_SOURCE_DIR}/makefile.in"
  "${CMAKE_BINARY_DIR}/package/makefile"
  @ONLY
  )

