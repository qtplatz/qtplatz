#
cmake_minimum_required(VERSION 2.8.11)
project( wix_installer )

execute_process( COMMAND git describe --abbrev=4 HEAD
  COMMAND sed -e "s/[\\.-]/,/g"
  OUTPUT_VARIABLE pod_version
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

#set( pod_version "v3,1,4" )
message( "####### pod version: " ${pod_version} )

string( REGEX MATCHALL "[0-9]+|,[0-9]+" version_list ${pod_version} )
list( LENGTH version_list count )

message( "##### version count: " ${count} )
if ( ( count GREATER 1 ) AND ( count LESS 3 ) )
  string( REGEX REPLACE "v([0-9]+)\\,([0-9]+).*$" "\\1;\\2;0;0" version_numbers ${pod_version} )
endif()
if ( ( count GREATER 2 ) AND ( count LESS 4 ) )
  string( REGEX REPLACE "v([0-9]+)\\,([0-9]+),([0-9]+).*$" "\\1;\\2;\\3;0" version_numbers ${pod_version} )
endif()
if ( count GREATER 3 )
  string( REGEX REPLACE "v([0-9]+)\\,([0-9]+),([0-9]+),([0-9]+).*$" "\\1;\\2;\\3;\\4" version_numbers ${pod_version} )
endif()

#"@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_MICRO@.@VERSION_PATCH@"
list( GET version_numbers 0 VERSION_MAJOR )
list( GET version_numbers 1 VERSION_MINOR )
list( GET version_numbers 2 VERSION_MICRO )
list( GET version_numbers 3 VERSION_PATCH )

set( MSIFILE qtplatz-${git_describe}.msi )

string( REGEX REPLACE "/" "\\\\" QTPLATZ_SOURCE_DIR ${QTPLATZ_SOURCE_DIR} )
message( "QTPLATZ_SOURCE_DIR: " ${QTPLATZ_SOURCE_DIR} )
  
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

add_custom_target( makefile DEPENDS makefile.in )


