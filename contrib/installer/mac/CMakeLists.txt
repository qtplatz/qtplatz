#
cmake_minimum_required(VERSION 3.18.2)
project( mac_installer )

include( qtplatz-version )

set( QTPLATZ_VERSION ${git_describe} )
set( RDKIT_LIBRARY_DIR ${RDKit_LIBRARY_DIRS} )
set( RDKIT_LIBRARIES )

set ( OSX_TARGET_ARCH )
list ( LENGTH CMAKE_OSX_ARCHITECTURES __arch_counts )

if ( __arch_counts GREATER "1" )
  set ( OSX_TARGET_ARCH "universal" )
elseif( __arch_counts EQUAL "1" )
  set ( OSX_TARGET_ARCH "${CMAKE_OSX_ARCHITECTURES}" )
elseif( __arch_counts EQUAL "0" )
  set ( OSX_TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}" )
endif()

message( STATUS "####### mac installer ######### --- " ${OSX_TARGET_ARCH} )
message( STATUS "####### mac installer QTPLATZ_VERSION  : ${QTPLATZ_VERSION}" )
message( STATUS "####### mac installer VERSION  : ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_PATCH}" )
message( STATUS "####### mac installer HASH     : ${VERSION_HASH}" )

set( QTPLATZ_SOURCE_DIR "${CMAKE_SOURCE_DIR}" )
set( QTPLATZ_BINARY_DIR "${CMAKE_BINARY_DIR}" )

configure_file(
  "${PROJECT_SOURCE_DIR}/makefile.in"
  "${CMAKE_BINARY_DIR}/package/Makefile"
  @ONLY
  )
