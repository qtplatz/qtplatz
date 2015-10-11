#
cmake_minimum_required(VERSION 2.8.11)
project( wix_installer )

set( QTPLATZ_BINARY_DIR "${CMAKE_BINARY_DIR}/../qtplatz.package" )

if ( QTPALTZ_SUPPORT_CORBA )
  find_package( ace+tao )
endif()

set( INFITOF_TARGET "" )
set( MALPIX_TARGET "" )

find_path( INFITOF_SOURCE_DIR NAMES "infitof-config.cmake" HINTS ${CMAKE_SOURCE_DIR}/../infitof )
if ( INFITOF_SOURCE_DIR )
  set( INFITOF_BINARY_DIR "${CMAKE_BINARY_DIR}/../infitof.package" )
  set( INFITOF_TARGET "infitof" )
endif()

find_path( MALPIX_SOURCE_DIR NAMES "src/mpx4" "src/plugins/malpix" HINTS ${CMAKE_SOURCE_DIR}/../malpix/malpix )
if ( MALPIX_SOURCE_DIR )
  set( MALPIX_BINARY_DIR "${CMAKE_BINARY_DIR}/../malpix.package" )
  set( MALPIX_TARGET "malpix" )
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

if ( QTPLATZ_SUPPORT_CORBA )
  string( REGEX REPLACE "/" "\\\\" ACE_ROOT ${ace+tao_DIR} )
endif()
  
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
  "${PROJECT_SOURCE_DIR}/rdkit_components.wxi"
  "${PROJECT_BINARY_DIR}/rdkit_components.wxi"
  @ONLY
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/tao_components.wxi"
  "${PROJECT_BINARY_DIR}/tao_components.wxi"
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


