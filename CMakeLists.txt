#
project (qtplatz)

cmake_minimum_required (VERSION 3.4)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

set ( CMAKE_ECLIPSE_VERSION 4.5 )
set ( DEBUG_SYMBOL OFF CACHE BOOL "DEBUG_SYMBOL" )
set ( QTPLATZ_CORELIB_ONLY OFF CACHE BOOL "QTPLATZ CORE LIBRARY ONLY" )
set ( QTPLATZ_SUPPORT_CORBA OFF CACHE BOOL "SUPPORT CORBA" )

if ( WIN32 )
  set( CMAKE_INSTALL_PREFIX "C:/QtPlatz" )
elseif( APPLE )
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
else()
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )  
endif()

set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR} )
set( QTPLATZ_BINARY_DIR ${CMAKE_BINARY_DIR} )

# cofigure boost & compiler
include (qtplatz.cmake)
include (version)

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
  set( CPACK_INSTALL_PREFIX "C:/QtPlatz" )
  set( CPACK_SET_DESTDIR OFF )  # WiX does not support this, and raise an error
elseif ( APPLE )
  set( CPACK_INSTALL_PREFIX "/opt/QtPlatz.app" )
  set( CPACK_SET_DESTDIR ON )  
else() # Linux
  set( CPACK_INSTALL_PREFIX "/opt/qtplatz" )
  set( CPACK_SET_DESTDIR ON )
endif()

if ( WIN32 )
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

if ( APPLE )
  set(CMAKE_DEBUG_POSTFIX "_debug")
endif()

message( STATUS "---------------------------------" )

if ( WIN32 )
  add_definitions( -wd4251 -wd4244 -wd4005 )
endif()

find_package( Boost REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

if ( NOT QTPLATZ_CORELIB_ONLY )
  if ( ( MSVC_VERSION GREATER 1900 ) OR ( MSVC_VERSION EQUAL 1900 ) )
    find_package( Qt5 5.6 OPTIONAL_COMPONENTS Core QUIET )
  else()
    set( Qt5_DIR $ENV{QTDIR}/lib/cmake/Qt5 )
    find_package( Qt5 5.5 OPTIONAL_COMPONENTS Core QUIET )
  endif()

  if ( Qt5_FOUND )
    find_package( qwt )
    message( STATUS "Found the QWT library at " ${qwt_DIR} )
  else()
    message( STATUS "Qt5 NOT Found while NOT QTPLATZ_CORELIB_ONLY" )
    message( STATUS "##### Qt5 NOT Found while NOT QTPLATZ_CORELIB_ONLY" )
    set( QTPLATZ_CORELIB_ONLY 1 )
  endif()
endif()

set( RDBASE "" )
find_package( rdkit QUIET )
if ( rdkit_FOUND )
  add_definitions( -DHAVE_RDKit=1 )
else()
  message( STATUS "No RDKit found" )
endif()

message( STATUS "---------------------------------" )


configure_file(
  ${QTPLATZ_SOURCE_DIR}/qtplatz-config.cmake.in
  ${QTPLATZ_BINARY_DIR}/qtplatz-config.cmake @ONLY )

configure_file(
  ${QTPLATZ_SOURCE_DIR}/doxyfile.in
  ${QTPLATZ_BINARY_DIR}/Doxyfile )

if ( WIN32 )
  set( SOLUTION_FILE "qtplatz" )  
  configure_file(
    ${QTPLATZ_SOURCE_DIR}/makefile.nmake.in
    ${CMAKE_BINARY_DIR}/makefile @ONLY )

  if ( DEBUG_SYMBOL )
    set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DEBUG" )
    set( CMAKE_MODULE_LINKER_FLAGS_RELEASE "/DEBUG" )
    set( CMAKE_CXX_FLAGS_RELEASE "/MD /Zi /O2 /Ob2 /D NDEBUG" )
  endif()

endif()

install( FILES ${QTPLATZ_BINARY_DIR}/qtplatz-config.cmake DESTINATION / COMPONENT libraries )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

add_definitions(-DUNICODE -D_UNICODE)

message( STATUS "------------ src ----------------" )
add_subdirectory (src)
message( STATUS "------------ contrib ----------------" )
add_subdirectory (contrib)
if ( Qt5_FOUND )
  message( STATUS "------------ share ----------------" )
  add_subdirectory (share)
endif()

########### CPack ########

message( STATUS "------------ cpack ----------------" )
set( CPACK_PACKAGE_NAME "qtplatz" )

include( qtplatz-cpack-config )

if ( WIN32 )
  
  set( CPACK_GENERATOR "WIX" )
  set( CPACK_WIX_UPGRADE_GUID "F8077E17-B0F6-4DDB-A7D9-CD7578199E82" )
  set( CPACK_WIX_PROGRAM_MENU_FOLDER "QtPlatz Mass Spectrometry" )
  set( CPACK_WIX_CMAKE_PACKAGE_REGISTRY "QtPlatz" )
  
  #  install( FILES
  #    $ENV{VCINSTALLDIR}/redist/x64/Microsoft.VC120.CRT/msvcp120.dll
  #    $ENV{VCINSTALLDIR}/redist/x64/Microsoft.VC120.CRT/msvcr120.dll
  #    $ENV{VCINSTALLDIR}/redist/x64/Microsoft.VC120.CRT/vccorlib120.dll
  #    DESTINATION bin COMPONENT runtime_libraries )
  #    )
  include( boost-cpack )
  include( qt5-cpack )  

elseif( APPLE )

  SET( MACOSX_BUNDLE_STARTUP_COMMAND qtplatz)
  set( CPACK_BINARY_DRAGNDROP ON )
  include( qtplatz-cpack-config )
  include( boost-cpack )
  include( qt5-cpack )
  include( rdkit-cpack )
  
elseif ( ${CMAKE_SYSTEM} MATCHES "Linux" )

  set( CPACK_DEB_COMPONENT_INSTALL 1 )

  set( CPACK_GENERATOR "DEB" )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst;${CMAKE_CURRENT_BINARY_DIR}/prerm" )
  set( CPACK_DEBIAN_PACKAGE_DESCRIPTION "QtPlatz -- Mass spectrometry data system desktop application" )

  if ( RTC_ARCH_ARM )
    set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
  endif()

  set( INSTALL_DIRECTORY ${CPACK_INSTALL_PREFIX}/lib )  
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/postinst.in ${CMAKE_CURRENT_BINARY_DIR}/postinst @ONLY )
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/prerm.in ${CMAKE_CURRENT_BINARY_DIR}/prerm @ONLY )  
  
  include( qtplatz-cpack-config )
  include( boost-cpack )
  if ( NOT QTPLATZ_CORELIB_ONLY )
    if ( Qt5_FOUND )
      include( qt5-cpack )
    endif()
    include( rdkit-cpack )
  endif()
endif()

set( CPACK_CREATE_DESKTOP_LINKS QtPlatz )
set( CPACK_PACKAGE_EXECUTABLES QtPlatz "QtPlatz")

include( CPack )

message( STATUS "---------- SUMMARY ------------" )
message( STATUS "# QTPLATZ_BINARY_DIR    : " ${QTPLATZ_BINARY_DIR} )
message( STATUS "# QTPLATZ_SUPPORT_CORBA : " ${QTPLATZ_SUPPORT_CORBA} )
message( STATUS "# QTPLATZ_CORELIB_ONLY  : " ${QTPLATZ_CORELIB_ONLY} )
message( STATUS "# DEBUG SYMBOL          : " ${DEBUG_SYMBOL} )
if ( WIN32 )
message( STATUS "# MSVC VERSION          : " ${MSVC_VERSION} )      
endif()
message( STATUS "# Boost_INCLUDE_DIRS    : " ${Boost_INCLUDE_DIRS} )
message( STATUS "# Boost_LIBRARY_DIRS    : " ${Boost_LIBRARY_DIRS} )
message( STATUS "# Qt5_DIR               : " ${Qt5_DIR} )
message( STATUS "# QTDIR                 : " ${QTDIR} )
message( STATUS "# QMAKE                 : " ${QMAKE} )
message( STATUS "# QWT DIR               : " ${qwt_DIR} )      
message( STATUS "# XMLPATTERNS           : " ${XMLPATTERNS} )
message( STATUS "# RDKit                 : " ${rdkit_FOUND} )
if ( rdkit_FOUND )
  message( STATUS "# RDKit VERSION         : " ${RDKit_PACKAGE_VERSION} )
#  foreach ( _dir ${RDKit_LIBRARIES} )
#    message( STATUS "    # RDKit LIBRARY     : " ${_dir} )
#  endforeach()  
  foreach ( _dir ${RDKit_INCLUDE_DIRS} )
    message( STATUS "    # RDKit INCLUDE     : " ${_dir} )
  endforeach()
endif()
message( STATUS "# TARGET_ARCHITECTURE  : "  ${__arch} )
message( STATUS "# PACKAGE_ARCHITECTURE : "  ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE} )
message( STATUS "# CMAKE_BUILD_TYPE     : "  ${CMAKE_BUILD_TYPE} )
message( STATUS "# CMAKE_GENERATOR      : "  ${CMAKE_GENERATOR} )
message( STATUS "---------- END SUMMARY ------------" )
