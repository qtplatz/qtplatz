cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

project (qt${QT_VERSION_MAJOR}-dev)

if ( CMAKE_SYSTEM_NAME MATCHES Linux )

  set( CPACK_GENERATOR DEB )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_VERSION "${VERSION}" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst" )

  if ( CMAKE_CROSSCOMPILING )
    # relative to root ex '/opt/helio/arm-linux-gnueabihf/usr/local/qt5'
    set( CMAKE_PREFIX_PATH /usr/local/qt${QT_VERSION_MAJOR} /opt/qt${QT_VERSION_MAJOR}pi )
    include(CheckSymbolExists)
    check_symbol_exists( "__arm__" "" RTC_ARCH_ARM )
    if ( RTC_ARCH_ARM )
      set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
    endif()
  else()
    return()  # do nothing for native build -- usually, binary installer is avilable
  endif()

  set( CPACK_SET_DESTDIR ON )
  if ( "${CPACK_INSTALL_PREFIX}" STREQUAL "" )
    set( CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
  endif()

  set( TARGET_LIBRARY_DIRECTORY ${CPACK_INSTALL_PREFIX}/qt${QT_VERSION_MAJOR}/lib )

  # replace TARGET_LIBRARY_DIRECTORY with actual value at install-destination
  configure_file( ${CMAKE_SOURCE_DIR}/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY )

endif()

message( "#### CMAKE_PREFIX_PATH " ${CMAKE_PREFIX_PATH} )
#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets )

get_filename_component( _dir "${Qt${QT_VERSION_MAJOR}_DIR}/../../.." REALPATH )
message( "##### install prefix: "    ${CMAKE_INSTALL_PREFIX} ";" ${CPACK_INSTALL_PREFIX} )
message( "##### library directory: " ${TARGET_LIBRARY_DIRECTORY} )
message( "##### source directory: "  ${_dir} )

install( DIRECTORY ${_dir} DESTINATION . )

set( CPACK_PACKAGE_NAME "qt${QT_VERSION_MAJOR}-dev" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${QT_VERSION} )
set( CPACK_PACKAGE_VERSION_MAJOR ${QT_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${QT_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${QT_VERSION_PATCH} )

#set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )

include( CPack )
