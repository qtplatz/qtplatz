#
cmake_minimum_required(VERSION 2.8.11)
project( wix_installer )

find_path( INFITOF_SOURCE_DIR NAMES "infitof-config.cmake" HINTS ${QTPLATZ_SOURCE_DIR}/../infitof )

include( qtplatz-version )

set( MSIFILE qtplatz-${git_describe}.msi )
set( STAGEDIR ${PROJECT_BINARY_DIR}/stage )

string( REGEX REPLACE "/" "\\\\" QTPLATZ_SOURCE_DIR ${QTPLATZ_SOURCE_DIR} )
string( REGEX REPLACE "/" "\\\\" INFITOF_SOURCE_DIR ${INFITOF_SOURCE_DIR} )
string( REGEX REPLACE "/" "\\\\" STAGEDIR ${STAGEDIR} )
  
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
  "${PROJECT_SOURCE_DIR}/qtplatz.wxs"
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

configure_file(
  "${PROJECT_SOURCE_DIR}/tao_components.wxi"
  "${PROJECT_BINARY_DIR}/tao_components.wxi"
  @ONLY
  )

file( GLOB XSLTFILES ${PROJECT_SOURCE_DIR}/*.xsl )
file( COPY ${XSLTFILES} DESTINATION ${PROJECT_BINARY_DIR} )

foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS} ${Qt5Sql_PLUGINS} )
  get_target_property( _loc ${plugin} LOCATION )
  file( RELATIVE_PATH _rname $ENV{QTDIR}/plugins ${_loc} )
  get_filename_component(_rpath ${_rname} DIRECTORY )
  file( COPY ${_loc} DESTINATION ${STAGEDIR}/plugins/${_rpath} )
endforeach()

add_custom_target( makefile DEPENDS makefile.in )


