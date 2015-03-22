cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

project (qt5-dev)

if ( "${QTPLATZ_SOURCE_DIR}" STREQUAL "" )

  set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../.. )
  include( ${QTPLATZ_SOURCE_DIR}/qtplatz.cmake )

  if ( RTC_ARCH_ARM )
    set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
  endif()

endif()

if ( CMAKE_SYSTEM_NAME MATCHES Linux )
  set( CMAKE_PREFIX_PATH /usr/local/qt5 /opt/qt5pi )
  set( CPACK_GENERATOR DEB )
endif()

find_package( Qt5 CONFIG REQUIRED Core Widgets )

get_filename_component( _dir ${Qt5_DIR}/../../.. REALPATH )
message( "## Qt5_DIR: " ${_dir} )

install( DIRECTORY ${_dir} DESTINATION . )

set( TARGET_LIBRARY_DIRECTORY ${CPACK_INSTALL_PREFIX}/qt5/lib )
configure_file( ${CMAKE_SOURCE_DIR}/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY ) 

set( CPACK_PACKAGE_NAME "qt5-dev" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${Qt5_VERSION} )
set( CPACK_PACKAGE_VERSION_MAJOR ${Qt5_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${Qt5_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${Qt5_VERSION_PATCH} )

set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
set( CPACK_DEBIAN_PACKAGE_VERSION "${VERSION}" )
set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst" )

set( CPACK_SET_DESTDIR ON )
set( CPACK_INSTALL_PREFIX ${INSTALL_DIRECTORY} )

include( CPack )

