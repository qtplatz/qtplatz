
project (tao)

cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

include(CheckSymbolExists)

set( ace+tao_DIR /opt/raspi/arm-linux-gnueabihf/usr/local/ace+tao/6.3.0 )

set( ACE_INCLUDE_DIR ${ace+tao_DIR}/include ) # this can be "$ENV{ACE_ROOT}" (ACE_wrappers)
set( TAO_INCLUDE_DIR ${ace+tao_DIR}/include ) # this can be "$ENV{TAO_ROOT}" (ACE_wrappers/TAO)

file( STRINGS ${ACE_INCLUDE_DIR}/ace/Version.h ace_version REGEX "^#define[ \t]+ACE_VERSION" )
string( REGEX REPLACE "#define[ \t]*ACE_VERSION[ \t]+\"(.*)\"" "\\1" ACE_VERSION ${ace_version} )

file( STRINGS ${TAO_INCLUDE_DIR}/tao/Version.h tao_version REGEX "^#define[ \t]+TAO_VERSION" )
string( REGEX REPLACE "#define[ \t]*TAO_VERSION[ \t]+\"(.*)\"" "\\1" TAO_VERSION ${tao_version} )

message( "## ACE_VERSION : " ${ACE_VERSION} )
message( "## TAO_VERSION : " ${TAO_VERSION} )

execute_process( COMMAND file ${ace+tao_DIR}/bin/tao_idl
  RESULT_VARIABLE file_result
  OUTPUT_VARIABLE file_output
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if ( file_result )
  message( FATAL_ERROR "execute 'file tao_idl' failed.  No target archtecute identified." )
endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

  set( CPACK_GENERATOR DEB )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_VERSION "${ACE_VERSION}" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst" )

  if ( file_output MATCHES "ARM" )
    set( CPACK_INSTALL_PREFIX /opt/arm-linux-gnueabihf/usr/local/ace+tao/${ACE_VERSION} )
    set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
  else()
    set( CPACK_INSTALL_PREFIX /opt/arm-linux-gnueabihf/usr/local/ace+tao/${ACE_VERSION} )    
  endif()
  
endif()

message( "## CPACK_INSTALL_PREFIX : " ${CPACK_INSTALL_PREFIX} )

if ( WIN32 )
  set( CPACK_GENERATOR WIX )
  set( CPACK_WIX_UPGRADE_GUID 93AE17E8-7535-4355-9AEC-8A32DE50BC63 )
endif()

set( INSTALL_DIRECTORY ${CPACK_INSTALL_PREFIX} )
configure_file( ${CMAKE_SOURCE_DIR}/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY ) 

set( CPACK_GENERATOR DEB )
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
set( CPACK_DEBIAN_PACKAGE_VERSION ${ACE_VERSION} )
set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst" )

set( CPACK_PACKAGE_NAME "ace+tao" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${ACE_VERSION} )

set( CPACK_SET_DESTDIR ON )

install( DIRECTORY ${ace+tao_DIR}/ DESTINATION . )

include( CPack )
