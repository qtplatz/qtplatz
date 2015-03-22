cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

project (boost)

find_package( Boost 1.57 REQUIRED )

if ( WIN32 )
  file( GLOB _libs "${Boost_LIBRARY_DIRS}/libboost_*.dll" )
elseif( APPLE )
  file( GLOB _libs "${Boost_LIBRARY_DIRS}/libboost_*.dylib" )  
else()
  file( GLOB _libs RELATIVE "${Boost_LIBRARY_DIRS}" "${Boost_LIBRARY_DIRS}/libboost_*.so*" )
endif()

#set( CPACK_OUTPUT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CPackConfig.cmake" )

if ( CMAKE_SYSTEM_NAME MATCHES Linux )

  set( CPACK_GENERATOR DEB )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_VERSION "${VERSION}" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst" )
  if ( RTC_ARCH_ARM )
    set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
  endif()
#  set( CPACK_SET_DESTDIR ON )
#  set( CPACK_INSTALL_PREFIX "/opt/lib" )

  set( INSTALL_DIRECTORY ${CPACK_INSTALL_PREFIX}/lib )  
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/postinst.in ${CMAKE_CURRENT_BINARY_DIR}/postinst @ONLY )

endif()

message( "##### boost install_directory: " ${INSTALL_DIRECTORY} )
message( "##### boost source directory: " ${Boost_LIBRARY_DIRS} )
message( "##### boost version: " ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )

set( CPACK_PACKAGE_NAME "boost" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
set( CPACK_PACKAGE_VERSION_MAJOR ${Boost_MAJOR_VERSION} )
set( CPACK_PACKAGE_VERSION_MINOR ${Boost_MINOR_VERSION} )
set( CPACK_PACKAGE_VERSION_PATCH ${Boost_SUBMINOR_VERSION} )

set( CPACK_SET_DESTDIR ON )

message( "Boost install: " ${Boost_LIBRARIES} " to " "${CMAKE_INSTALL_PREFIX}/lib" )

foreach( lib ${_libs} )
  # assume distination to be: /opt/qtplatz/lib
  install( FILES ${Boost_LIBRARY_DIRS}/${lib} DESTINATION lib COMPONENT runtime_libraries )
endforeach()

include( CPack )
