#
cmake_minimum_required(VERSION 3.18.2)
project( mac_installer )

include( qtplatz-version )

set( QTPLATZ_VERSION ${git_describe} )
set( RDKIT_LIBRARY_DIR ${RDKit_LIBRARY_DIRS} )
set( RDKIT_LIBRARIES )

message( STATUS "####### mac installer #########" )
message( STATUS "####### Boost_LIBRARY_DIRS : " ${Boost_LIBRARY_DIRS} )
message( STATUS "####### Python3_VERSION : " ${Python3_VERSION} )

set( QTPLATZ_SOURCE_DIR "${CMAKE_SOURCE_DIR}" )
set( QTPLATZ_BINARY_DIR "${CMAKE_BINARY_DIR}" )

configure_file(
  "${PROJECT_SOURCE_DIR}/makefile.in"
  "${CMAKE_BINARY_DIR}/package/Makefile"
  @ONLY
  )
