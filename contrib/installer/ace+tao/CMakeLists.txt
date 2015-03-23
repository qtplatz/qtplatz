cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

project (ace+tao-package)

if ( "${QTPLATZ_SOURCE_DIR}" STREQUAL "" )
  set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../.. )
  include( ${QTPLATZ_SOURCE_DIR}/qtplatz.cmake )
endif()

find_package( ace+tao )

message( "## ace+tao binary installer: " ${ace+tao_DIR} )
message( "## ACE_VERSION : " ${ACE_VERSION} " in directory " ${ACE_INCLUDE_DIR} )
message( "## TAO_VERSION : " ${TAO_VERSION} " in directory " ${TAO_INCLUDE_DIR} )

if ( WIN32 )
  file( GLOB _libs RELATIVE "${ace+tao_DIR}"
    "${ace+tao_DIRS}/*.dll" )
elseif( APPLE )
  file( GLOB _libs RELATIVE "${ace+tao_DIR}"
    "${ACE+TAO_LIBRARY_DIR}/libACE_*.dylib*" "${ACE+TAO_LIBRARY_DIR}/libTAO_*.dylib*" )
else()
  file( GLOB _libs RELATIVE "${ace+tao_DIR}"
    "${ACE+TAO_LIBRARY_DIR}/libACE_*.so*" "${ACE+TAO_LIBRARY_DIR}/libTAO_*.so*" )  
endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

  if ( CMAKE_CROSSCOMPILING ) 
    execute_process( COMMAND file ${ace+tao_DIR}/bin/tao_idl
      RESULT_VARIABLE file_result
      OUTPUT_VARIABLE file_output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    
    if ( file_result )
      message( FATAL_ERROR "execute 'file tao_idl' failed.  No target archtecute identified." )
    endif()
    if ( file_output MATCHES "ARM" )
      set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
    endif()
  endif()

  set( CPACK_GENERATOR DEB )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_VERSION "${ACE_VERSION}" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst" )

  set( CPACK_SET_DESTDIR ON )
  if ( "${CPACK_INSTALL_PREFIX}" STREQUAL "" )
    set( CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
  endif()
  
  set( TARGET_LIBRARY_DIRECTORY ${CPACK_INSTALL_PREFIX}/lib )    

  # replace TARGET_LIBRARY_DIRECTORY with actual value at install-destination
  configure_file( ${CMAKE_SOURCE_DIR}/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY )

endif()

if ( WIN32 )
  set( CPACK_GENERATOR WIX )
  set( CPACK_WIX_UPGRADE_GUID 93AE17E8-7535-4355-9AEC-8A32DE50BC63 )
endif()

set( CPACK_PACKAGE_NAME "ace+tao" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${ACE_VERSION} )

include( CPackComponent )
cpack_add_component( runtime_libraries DISPLAY_NAME "Runtime libraries" DESCRIPTION "libraries" GROUP Runtime )
cpack_add_component( libraries         DISPLAY_NAME "Development libraries" DESCRIPTION "static and import libs" GROUP Development )
cpack_add_component( headers           DISPLAY_NAME "C++ Headers" DESCRIPTION "C++ header files" GROUP Development )

foreach( _lib ${_libs} )
  # message( "install : " ${_lib} " --> " "${CMAKE_INSTALL_PREFIX}/${_lib}" )
  install( FILES ${ace+tao_DIR}/${_lib} DESTINATION lib COMPONENT runtime_libraries  )
endforeach()

include( CPack )
