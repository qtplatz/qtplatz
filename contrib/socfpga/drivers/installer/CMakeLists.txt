#
cmake_minimum_required (VERSION 2.8.9)

project ( dgmod )

set( CPACK_PACKAGE_NAME dgmod )
set( CPACK_PACKAGING_INSTALL_PREFIX "/" )
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../../../cmake/Modules )

include( "version" )
find_package( arch )

set( CPACK_PACKAGE_VENDOR "MS-Cheminformatics LLC" )
set( CPACK_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )
set( CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH} )
#set( CPACK_RESOURCE_FILE_LICENSE ${QTPLATZ_SOURCE_DIR}/license.rtf )

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  configure_file( "${PROJECT_SOURCE_DIR}/postinst.in" "${PROJECT_BINARY_DIR}/postinst" @ONLY )
  configure_file( "${PROJECT_SOURCE_DIR}/prerm.in" "${PROJECT_BINARY_DIR}/prerm" @ONLY )
endif()
  
include( CPackComponent )

find_file( init_d "init.d-dgmod.sh" PATHS "${CMAKE_SOURCE_DIR}/../dgmod" )
find_file( kmodule "dgmod.ko" PATHS "${CMAKE_SOURCE_DIR}/../dgmod" )

configure_file ( "${init_d}"  "${PROJECT_BINARY_DIR}/etc/init.d/dgmod" )

cpack_add_component( linux-modules     DISPLAY_NAME "linux-modules" GROUP linux-modules )
cpack_add_component( dgmod-module      DISPLAY_NAME "dgmod-module" GROUP dgmod )

cpack_add_component_group( linux-modules )
cpack_add_component_group( dgmod )

#set( CPACK_ALL_INSTALL_TYPES Full Developer)
#set( CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Developer Full )
#set( CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full )
#set( CPACK_COMPONENT_APPLICATIONS_INSTALL_TYPES Full )

#message( STATUS "CPack --- git-describe: " ${git_describe} )

#set( CPACK_DEB_COMPONENT_INSTALL 1 )

set( CPACK_GENERATOR "DEB" )
set( CPACK_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  "${PROJECT_BINARY_DIR}/prerm;${PROJECT_BINARY_DIR}/postinst" )

if ( RTC_ARCH_ARM )
  set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
endif()

INSTALL( DIRECTORY ${PROJECT_BINARY_DIR}/etc DESTINATION . COMPONENT dgmod )
INSTALL( FILES ${kmodule} DESTINATION "lib/modules/4.4.6/extra" COMPONENT dgmod )

include( CPack )
