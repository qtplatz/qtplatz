# CMakeLists files in this project malpix4
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.

cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

project (bnc565-httpd)
set ( CMAKE_ECLIPSE_VERSION 4.5 )

include ( standalone.cmake )

option( DEBUG_SYMBOL "DEBUG_SYMBOL" OFF )

if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL "" )
  # WIN32 has no default BUILD_TYPE
  string( TOLOWER ${CMAKE_BUILD_TYPE} build_type )
else()
  set( build_type "release" )
endif()

#set( QTPLATZ_BUILD_DIR ${CMAKE_BINARY_DIR}/../qtplatz.${build_type} CACHE STRING "destination directory" )

include ( version )
include_directories( ${PROJECT_BINARY_DIR} )

set ( VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )
set ( HTML_INSTALL_PREFIX "/opt" )
set ( HTML_INSTALL_DIR "${HTML_INSTALL_PREFIX}/httpd" )
set ( CMAKE_INSTALL_PREFIX "/usr/local" )    # override qtplatz-config.cmake
set ( CPACK_INSTALL_PREFIX "/opt/httpd" ) # override qtplatz-config.cmake

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
  
set( QTPLATZ_INSTALL_PREFIX ${CPACK_INSTALL_PREFIX} ) # /usr/local
set( CMAKE_INSTALL_PREFIX ${CPACK_INSTALL_PREFIX} ) # /usr/local

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY} ) # static libs, dll's import lib/exp
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY} ) # plugins
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY} ) # plugins
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY} ) # plugins
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_RUNTIME_OUTPUT_DIRECTORY} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_RUNTIME_OUTPUT_DIRECTORY} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_RUNTIME_OUTPUT_DIRECTORY} )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

find_package( Boost 1.57 REQUIRED )

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory( httpd )

set( CPACK_PACKAGE_VERSION ${VERSION} )
set( CMAKE_INSTALL_NAME_DIR ./bin )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

link_directories(${Boost_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib)

install ( DIRECTORY html DESTINATION ${HTML_INSTALL_DIR} COMPONENT httpd )
install( FILES ${DEPENDENT_LIBRARIES} DESTINATION bin COMPONENT applications )

######## CPack
#
#if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
#
#  set( CPACK_GENERATOR DEB )
#  set( CPACK_DEB_COMPONENT_INSTALL 1 )
#  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
#  set( HTTPD_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/httpd" )
#  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${HTTPD_BINARY_DIR}/postinst;${HTTPD_BINARY_DIR}/prerm" )  
#  set( CPACK_DEBIAN_HTTPD_PACKAGE_CONTROL_EXTRA "${HTTPD_BINARY_DIR}/postinst;${HTTPD_BINARY_DIR}/prerm" )
#  set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${__arch} )
# endif()

#include( qtplatz-cpack-config )
#cpack_add_component( httpd DISPLAY_NAME "Delay pulse controller" DESCRIPTION "web server" GROUP httpd )
