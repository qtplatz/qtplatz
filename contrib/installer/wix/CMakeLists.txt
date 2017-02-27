#
cmake_minimum_required(VERSION 2.8.11)
project( wix_installer )

set ( BDBASE "C:/Program Files (x86)/Bruker Daltonik/CompassXtract/3.2.201 (64 bit)" )
set ( INFITOF_TARGET "" )
set ( MALPIX_TARGET "" )

if ( MSVC_VERSION EQUAL 1900 )
  set( Generator "Visual Studio 14 Win64" )
  set( vcvers "vc140" )
elseif( MSVC_VERSION EQUAL 1800 )
  set( Generator "Visual Studio 12 Win64" )
  set( vcvers "vc120" )
endif()


find_path( INFITOF_SOURCE_DIR NAMES "infitof-config.cmake" HINTS ${CMAKE_SOURCE_DIR}/../infitof )
if ( INFITOF_SOURCE_DIR )
  set( INFITOF_TARGET "infitof" )
  if ( MSVC_VERSION EQUAL 1900 )
    set( INFITOF_BINARY_DIR "${CMAKE_BINARY_DIR}/../${INFITOF_TARGET}.package_vc14" )
  elseif( MSVC_VERSION EQUAL 1800 )
    set( INFITOF_BINARY_DIR "${CMAKE_BINARY_DIR}/../${INFITOF_TARGET}.package_vc12" )    
  endif()  
endif()

find_path( MALPIX_SOURCE_DIR NAMES "src/plugins/malpixacquire" HINTS ${CMAKE_SOURCE_DIR}/../malpix/malpix )
if ( MALPIX_SOURCE_DIR )
  set( MALPIX_TARGET "malpix" )
  if ( MSVC_VERSION EQUAL 1900 )
    set( MALPIX_BINARY_DIR "${CMAKE_BINARY_DIR}/../${MALPIX_TARGET}.package_vc14" )
  elseif( MSVC_VERSION EQUAL 1800 )
    set( MALPIX_BINARY_DIR "${CMAKE_BINARY_DIR}/../${MALPIX_TARGET}.package_vc12" )
  endif()
endif()

include( qtplatz-version )

set( MSIFILE qtplatz-${git_describe}.msi )
set( STAGEDIR ${PROJECT_BINARY_DIR}/stage )

string( REGEX REPLACE "/" "\\\\" QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR} )
string( REGEX REPLACE "/" "\\\\" QTPLATZ_BINARY_DIR ${QTPLATZ_BINARY_DIR} )

if ( INFITOF_SOURCE_DIR AND INFITOF_BINARY_DIR )
  string( REGEX REPLACE "/" "\\\\" INFITOF_SOURCE_DIR ${INFITOF_SOURCE_DIR} )
  string( REGEX REPLACE "/" "\\\\" INFITOF_BINARY_DIR ${INFITOF_BINARY_DIR} )
endif()

if ( MALPIX_SOURCE_DIR AND MALPIX_BINARY_DIR )
  string( REGEX REPLACE "/" "\\\\" MALPIX_SOURCE_DIR ${MALPIX_SOURCE_DIR} )
  string( REGEX REPLACE "/" "\\\\" MALPIX_BINARY_DIR ${MALPIX_BINARY_DIR} )
endif()

string( REGEX REPLACE "/" "\\\\" STAGEDIR ${STAGEDIR} )

string( REGEX REPLACE "/" "\\\\" QTDIR ${QTDIR} )

  
configure_file(
  "${PROJECT_SOURCE_DIR}/makefile.in"
  "${PROJECT_BINARY_DIR}/makefile"
  @ONLY
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/version.wxi.in"
  "${PROJECT_BINARY_DIR}/version.wxi"
  @ONLY
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/qtplatz.wxs.in"
  "${PROJECT_BINARY_DIR}/qtplatz.wxs"
  @ONLY
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/rdkit_components.wxi.in"
  "${PROJECT_BINARY_DIR}/rdkit_components.wxi"
  @ONLY
  )

file( GLOB XSLTFILES ${PROJECT_SOURCE_DIR}/*.xslt )
file( COPY ${XSLTFILES} DESTINATION ${PROJECT_BINARY_DIR} )

find_package( Qt5 REQUIRED Gui Svg Sql )

foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS} ${Qt5Sql_PLUGINS} )
  get_target_property( _loc ${plugin} LOCATION )
  file( RELATIVE_PATH _rname ${QTDIR}/plugins ${_loc} )
  get_filename_component(_rpath ${_rname} DIRECTORY )
  file( COPY ${_loc} DESTINATION ${STAGEDIR}/plugins/${_rpath} )
endforeach()

add_custom_target( makefile DEPENDS makefile.in )


