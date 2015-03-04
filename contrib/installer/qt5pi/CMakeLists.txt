
project (qt5pi)

cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

set( rootfs /mnt/raspi-rootfs )
set( qt5pi_DIR ${rootfs}/opt/raspi/qt5pi )  # /mnt/raspi-rootfs/opt/raspi/qt5pi --> /opt/raspi/qt5pi
set( INSTALL_PREFIX /opt/raspi )
set( INSTALL_DIRECTORY ${INSTALL_PREFIX}/qt5pi )
set( qt5_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../../qt5 )

set( _major 5 )
set( _minor 4 )
set( _patch 1 )
set( _tweak 0 )

configure_file( ${CMAKE_SOURCE_DIR}/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY ) 

# Qt version from git
execute_process( COMMAND git describe
  WORKING_DIRECTORY ${qt5_SOURCE_DIR}
  RESULT_VARIABLE git_result
  OUTPUT_VARIABLE git_describe
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

message( "## git describe: " ${git_describe} )

string( REGEX MATCHALL "[0-9]+|[\\.\\-][0-9]+" _list ${git_describe} )
list( LENGTH _list _count )
if ( _count GREATER 1 ) # count >= 2
  string( REGEX REPLACE "v([0-9]+)\\.[0-9]+.*$" "\\1" _major ${git_describe} )
  string( REGEX REPLACE "v[0-9]+\\.([0-9]+).*$" "\\1" _minor ${git_describe} )
endif()
if ( _count GREATER 2 ) # count >= 3
  string( REGEX REPLACE "v[0-9]+\\.[0-9]+[\\.-]([0-9]+).*$" "\\1" _patch ${git_describe} )  
endif()
if ( _count GREATER 3 ) # count >= 4
  string( REGEX REPLACE "v[0-9]+\\.[0-9]+[\\.-][0-9]+-[A-Za-z0-9]*-([0-9]+)-.*$" "\\1" _tweak ${git_describe} )
endif()

message( "## making package version: " ${_major}.${_minor}.${_patch}.${_tweak} )

set( CPACK_GENERATOR DEB )
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
set( CPACK_DEBIAN_PACKAGE_VERSION "${VERSION}" )
set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst" )
set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )

set( CPACK_PACKAGE_NAME "qt5pi" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${_major}.${_minor}.${_patch}.${_tweak} )
set( CPACK_PACKAGE_VERSION_MAJOR ${_major} )
set( CPACK_PACKAGE_VERSION_MINOR ${_minor} )
set( CPACK_PACKAGE_VERSION_PATCH ${_patch} )
#set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/../../license.txt )

set( CPACK_SET_DESTDIR ON )
set( CPACK_INSTALL_PREFIX ${INSTALL_DIRECTORY} )

# install all files on /mnt/raspi-rootfs/opt/raspi/qt5pi to <dest>:/opt/raspi/qt5pi
install( DIRECTORY ${qt5pi_DIR}/ DESTINATION . )

include( CPack )
