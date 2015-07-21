#
project (qtplatz)

cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

set( QMAKE_LAYOUT CACHE BOOL 0 )
set( QTPLATZ_CORELIB_ONLY CACHE BOOL 0 )

if ( WIN32 )
  set( CMAKE_INSTALL_PREFIX "C:/qtplatz" )
elseif( APPLE )
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
else()
  set( CMAKE_INSTALL_PREFIX "/usr/local/qtplatz" )  
endif()

set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR} )
set( QTPLATZ_BINARY_DIR ${CMAKE_BINARY_DIR} )
set( CMAKE_BUILD_TYPE Release )

# cofigure boost & compiler
include (qtplatz.cmake)
include (qtplatz-version)

#
# Output directories layout, which is equivalent to original QTC layout
#

if ( APPLE )

  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib) # static libs, dll's import lib/exp

  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/PlugIns ) #plugins
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/PlugIns )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/PlugIns )

  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/MacOS )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/MacOS )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/MacOS )
  set( CMAKE_SHARED_MODULE_SUFFIX ".dylib" )
  set( CMAKE_MACOSX_RPATH 1 )
  
  set( QTPLATZ_PLUGIN_DIRECTORY ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/PlugIns )
  set( QTPLATZ_PLUGIN_DESTINATION bin/qtplatz.app/Contents/PlugIns )
  
else()

  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib) # static libs, dll's import lib/exp

  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/lib/qtplatz ) #plugins
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/lib/qtplatz )

  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin)  # .exe | .dll
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin )
  
  set( QTPLATZ_PLUGIN_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins )
  set( QTPLATZ_PLUGIN_DESTINATION lib/qtplatz/plugins )
  
  if ( CROSS_COMPILING )
    set( CMAKE_PREFIX_PATH /usr/local/qt5 )
  endif()

endif()

if( WIN32 )
  set( CPACK_INSTALL_PREFIX "C:/qtplatz" )
elseif ( APPLE )
  set( CPACK_INSTALL_PREFIX "/opt/qtplatz.app" )
  set( CPACK_SET_DESTDIR ON )  
else() # Linux
  set( CPACK_INSTALL_PREFIX "/opt/qtplatz" )
  set( CPACK_SET_DESTDIR ON )
endif()

configure_file(
  ${QTPLATZ_SOURCE_DIR}/qtplatz-config.cmake.in
  ${QTPLATZ_BINARY_DIR}/qtplatz-config.cmake @ONLY )

if ( WIN32 )
  set( SOLUTION_FILE "qtplatz" )  
  configure_file(
    ${QTPLATZ_SOURCE_DIR}/makefile.nmake.in
    ${CMAKE_BINARY_DIR}/makefile @ONLY )
#  set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DEBUG" )
#  set( CMAKE_MODULE_LINKER_FLAGS_RELEASE "/DEBUG" )
#  set( CMAKE_CXX_FLAGS_RELEASE "/MD /Zi /O2 /Ob2 /D NDEBUG" )
endif()

install( FILES ${QTPLATZ_BINARY_DIR}/qtplatz-config.cmake DESTINATION lib COMPONENT libraries )

find_package( Boost REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

find_package( Qt5 OPTIONAL_COMPONENTS Core QUIET )
if ( Qt5_FOUND )
  message( STATUS "Found the Qt5 libraries at " ${Qt5_DIR} )      
  find_package( qwt )
  message( STATUS "Found the QWT library at " ${qwt_DIR} )
endif()

find_package( rdkit QUIET )
if ( rdkit_FOUND )
  message( STATUS "Found the RDKit libraries at " ${RDKit_DIR} )
endif()

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

add_definitions(-DUNICODE -D_UNICODE)

if ( WIN32 )
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

add_subdirectory (src)
add_subdirectory (contrib)
if ( Qt5_FOUND )
  add_subdirectory (share)
endif()

########### CPack ########
if ( WIN32 )

  set( FATALERROR "CPACK_GENERATOR for Windows platform not supported." )

elseif( APPLE )

  SET(MACOSX_BUNDLE_STARTUP_COMMAND qtplatz)
  set(CPACK_BINARY_DRAGNDROP ON)
  
elseif ( ${CMAKE_SYSTEM} MATCHES "Linux" )

  set( CPACK_GENERATOR "DEB" )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst;${CMAKE_CURRENT_BINARY_DIR}/prerm" )  

  if ( RTC_ARCH_ARM )
    set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
  endif()

  set( INSTALL_DIRECTORY ${CPACK_INSTALL_PREFIX}/lib )  
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/postinst.in ${CMAKE_CURRENT_BINARY_DIR}/postinst @ONLY )
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/prerm.in ${CMAKE_CURRENT_BINARY_DIR}/prerm @ONLY )  
  
endif()

include( qtplatz-cpack-config )

include( boost-cpack )
include( ace+tao-cpack )

if ( Qt5_FOUND )
  include( qt5-cpack )
endif()

if ( rdkit_FOUND )
  include( rdkit-cpack )
endif()

set( CPACK_CREATE_DESKTOP_LINKS QtPlatz )

include( CPack )
