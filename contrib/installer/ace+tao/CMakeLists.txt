cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

project (ace+tao)

find_package( ace+tao )
message( "## ace+tao binary installer: " ${ace+tao_DIR} )
message( "## ACE_VERSION : " ${ACE_VERSION} " in directory " ${ACE_INCLUDE_DIR} )
message( "## TAO_VERSION : " ${TAO_VERSION} " in directory " ${TAO_INCLUDE_DIR} )

execute_process( COMMAND file ${ace+tao_DIR}/bin/tao_idl
  RESULT_VARIABLE file_result
  OUTPUT_VARIABLE file_output
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if ( file_result )
  message( FATAL_ERROR "execute 'file tao_idl' failed.  No target archtecute identified." )
endif()

if ( WIN32 )
  file( GLOB _libs RELATIVE "${ace+tao_DIR}" "${ace+tao_DIRS}/*.dll" )
elseif( APPLE )
  file( GLOB _libs RELATIVE "${ace+tao_DIR}" "${ACE+TAO_LIBRARY_DIR}/libACE_*.dylib*" "${ACE+TAO_LIBRARY_DIR}/libTAO_*.dylib*" )
else()
  file( GLOB _libs RELATIVE "${ace+tao_DIR}" "${ACE+TAO_LIBRARY_DIR}/libACE_*.so*" "${ACE+TAO_LIBRARY_DIR}/libTAO_*.so*" )  
endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

  set( CPACK_GENERATOR DEB )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_VERSION "${ACE_VERSION}" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst" )

#  set( CPACK_SET_DESTDIR ON )
#  set( CPACK_INSTALL_PREFIX "/opt/lib" )

  if ( file_output MATCHES "ARM" )
    set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
  endif()

  set( INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib )
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/postinst.in ${CMAKE_CURRENT_BINARY_DIR}/postinst @ONLY )
  
endif()

if ( WIN32 )
  set( CPACK_GENERATOR WIX )
  set( CPACK_WIX_UPGRADE_GUID 93AE17E8-7535-4355-9AEC-8A32DE50BC63 )
endif()

set( CPACK_PACKAGE_NAME "ace+tao" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${ACE_VERSION} )

foreach( _lib ${_libs} )
  message( "install : " ${_lib} " --> " "${CMAKE_INSTALL_PREFIX}/${_lib}" )
  install( FILES ${ace+tao_DIR}/${_lib} DESTINATION lib COMPONENT runtime_libraries  )
endforeach()

include( CPack )
