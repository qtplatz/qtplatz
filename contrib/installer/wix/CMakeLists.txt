#
cmake_minimum_required(VERSION 3.7)
project( wix_installer )

configure_file(
  "${PROJECT_SOURCE_DIR}/environment.wxs.in"
  "${PROJECT_BINARY_DIR}/environment.wxs"  @ONLY  )

#-------------------------
# configure_file(  "${PROJECT_SOURCE_DIR}/installdir.xml.in"  "${CMAKE_BINARY_DIR}/installdir.xml"  @ONLY  )
#-------------------------

set ( BDBASE "C:/Program Files (x86)/Bruker Daltonik/CompassXtract/3.2.201 (64 bit)" )
set ( INFITOF_TARGET "" )
set ( MALPIX_TARGET "" )

if ( MSVC_VERSION GREATER_EQUAL 1910 )
  set( Generator "Visual Studio 15 2017 Win64" )
  set( vcvers "vc141" )
  set( vsvers "vc15" )
elseif ( MSVC_VERSION EQUAL 1900 )
  message( FATAL_ERROR "VISUAL STUDIO VERSION NOT SUPPORTED" )
elseif( MSVC_VERSION EQUAL 1800 )
  message( FATAL_ERROR "VISUAL STUDIO VERSION NOT SUPPORTED" )
endif()

if ( OpenCV_FOUND )
  message( STATUS "##### OpenCV WIX: " ${OpenCV_ROOT_DIR} )
endif()

find_path( INFITOF_SOURCE_DIR NAMES "infitof-config.cmake" HINTS ${CMAKE_SOURCE_DIR}/../infitof )
if ( INFITOF_SOURCE_DIR )
  set( INFITOF_TARGET "infitof" )
  set( INFITOF_BINARY_DIR "${CMAKE_BINARY_DIR}/../${INFITOF_TARGET}.package" )
endif()

find_path( MALPIX_SOURCE_DIR NAMES "src/plugins/malpixacquire" HINTS ${CMAKE_SOURCE_DIR}/../malpix/malpix )
if ( MALPIX_SOURCE_DIR )
  set( MALPIX_TARGET "malpix" )
  set( MALPIX_BINARY_DIR "${CMAKE_BINARY_DIR}/../${MALPIX_TARGET}.package_${vsvers}" )
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

#find_package( Qt${QT_VERSION_MAJOR} REQUIRED Gui Svg Sql )

foreach(plugin ${QtGui_PLUGINS} ${QtSvg_PLUGINS} ${QtSql_PLUGINS} )
  get_target_property( _loc ${plugin} LOCATION )
  file( RELATIVE_PATH _rname ${QTDIR}/plugins ${_loc} )
  get_filename_component(_rpath ${_rname} DIRECTORY )
  file( COPY ${_loc} DESTINATION ${STAGEDIR}/plugins/${_rpath} )
endforeach()

add_custom_target( makefile DEPENDS makefile.in )
