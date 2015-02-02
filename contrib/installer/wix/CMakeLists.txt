#
cmake_minimum_required(VERSION 2.8.11)
project( wix_installer )

find_path( INFITOF_SOURCE_DIR NAMES "infitof-config.cmake" HINTS ${QTPLATZ_SOURCE_DIR}/../infitof )

include( qtplatz-version )

set( MSIFILE qtplatz-${git_describe}.msi )

string( REGEX REPLACE "/" "\\\\" QTPLATZ_SOURCE_DIR ${QTPLATZ_SOURCE_DIR} )
string( REGEX REPLACE "/" "\\\\" INFITOF_SOURCE_DIR ${INFITOF_SOURCE_DIR} )
  
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

file( GLOB XSLTFILES ${PROJECT_SOURCE_DIR}/*.xslt )
file( COPY ${XSLTFILES} DESTINATION ${PROJECT_BINARY_DIR} )

message("## WIX ## XSLTFILES : " ${XSLTFILES} )

add_custom_target( makefile DEPENDS makefile.in )


