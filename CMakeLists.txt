#
project (qtplatz)

cmake_minimum_required (VERSION 3.4)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

set ( CMAKE_ECLIPSE_VERSION 4.5 )
set ( DEBUG_SYMBOL OFF CACHE BOOL "DEBUG_SYMBOL" )

set ( QTPLATZ_CORELIB_ONLY OFF CACHE BOOL "QTPLATZ CORE LIBRARY ONLY" )

option( WITH_RDKIT         "Include chemistry support"  ON )
option( WITH_OPENCV        "Include OpenCV"             ON )
option( WITH_QT5           "Include QT5"                ON )
option( WITH_QWT           "Include QWT"                ON )
option( WITH_AqDrv4        "Keysight AP240/DC122"       ON )
option( WITH_AgMD2         "Keysight U5303A"            ON )

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
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/Frameworks )
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

  set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib/qtplatz )
  
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
  set( CMAKE_CXX_STANDARD 14 )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fext-numeric-literals" )
endif()

if ( WIN32 )
  set(CMAKE_DEBUG_POSTFIX "d")
endif()
if ( APPLE )
  set(CMAKE_DEBUG_POSTFIX "_debug")
endif()

if ( WIN32 )
  add_definitions( -wd4251 -wd4244 -wd4005 -wd4275 -wd4267 )
endif()

find_package( Boost 1.57 REQUIRED COMPONENTS bzip2 date_time iostreams system filesystem wserialization serialization locale regex program_options iostreams )
include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

if ( WITH_QWT )
  find_package( qwt )
  if ( NOT QWT_LIBRARIES )
    message( FATAL_ERROR "No QWT library found" )
  endif()
endif()

if ( WITH_RDKIT )
  find_package( rdkit QUIET )
  if ( rdkit_FOUND )
    add_definitions( -DHAVE_RDKit=1 )
  else()
    message( STATUS "No RDKit found" )
  endif()
endif()

if ( WITH_OPENCV )
  find_package( OpenCV 3 QUIET )
  find_package( HDF5 QUIET )
endif()

if ( WITH_AqDrv4 )
  find_package( AqDrv4 QUIET )
  if ( AqDrv4_FOUND )
    add_definitions( -DHAVE_AqDrv4=1 )
  endif()
endif()

if ( WITH_AgMD2 )
  find_package( AgMD2 QUIET )
  if ( AqMD2_FOUND )
    add_definitions( -DHAVE_AgMD2=1 )
  endif()  
endif()

#message( STATUS "---------------------------------" )

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

  file( WRITE "${CMAKE_BINARY_DIR}/build-package.bat" "msbuild /m:4 /p:Configuration=Release PACKAGE.vcxproj" )

  if ( DEBUG_SYMBOL )
    set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DEBUG" )
    set( CMAKE_MODULE_LINKER_FLAGS_RELEASE "/DEBUG" )
    set( CMAKE_CXX_FLAGS_RELEASE "/MD /Zi /O2 /Ob2 /D NDEBUG" )
  endif()

endif()

install( FILES ${QTPLATZ_BINARY_DIR}/qtplatz-config.cmake DESTINATION / COMPONENT libraries )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

add_definitions(-DUNICODE -D_UNICODE)
add_subdirectory (src)
add_subdirectory (contrib)
if ( Qt5_FOUND )
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

  include( boost-cpack )
  if ( Qt5_FOUND )
    include( qt5-cpack )
  endif()
  if ( rdkit_FOUND )
    include( rdkit-cpack )
  endif()
endif()

set( CPACK_CREATE_DESKTOP_LINKS QtPlatz )
set( CPACK_PACKAGE_EXECUTABLES QtPlatz "QtPlatz")

include( CPack )

message( STATUS "---------- SUMMARY ------------" )
message( STATUS "# QTPLATZ_BINARY_DIR    : " ${QTPLATZ_BINARY_DIR} )
message( STATUS "# QTPLATZ_CORELIB_ONLY  : " ${QTPLATZ_CORELIB_ONLY} )
message( STATUS "# DEBUG SYMBOL          : " ${DEBUG_SYMBOL} )
if ( WIN32 )
  message( STATUS "# MSVC VERSION          : " ${MSVC_VERSION} )      
endif()
message( STATUS "# Boost_VERSION         : " ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
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
  foreach ( _dir ${RDKit_LIBRARY_DIRS} )
    message( STATUS "\t# LIBRARY          : " ${_dir} )
  endforeach()
  if ( FALSE )
    foreach ( _dir ${RDKit_INCLUDE_DIRS} )
      message( STATUS "\t# INCLUDE          : " ${_dir} )
    endforeach()
    foreach( _name ${RDKit_LIBRARIES} )
      get_target_property( _value ${_name} IMPORTED_LOCATION )
      get_target_property( _value_debug ${_name} IMPORTED_LOCATION_DEBUG )
      message( STATUS "\t\t " ${_name} "\t" ${_value} "\n\t\t\t\t" ${_value_debug} )
    endforeach()
  endif()
endif()
message( STATUS "# OpenCV VERSION        : "  ${OpenCV_VERSION} )
if ( OpenCV_FOUND )
  foreach ( _dir ${OpenCV_INCLUDE_DIRS} )
    message( STATUS "\t# INCLUDE          : " ${_dir} )
  endforeach()
endif()
message( STATUS "# HDF5 VERSION          : " ${HDF5_VERSION} )
if ( HDF5_FOUND )
  foreach ( _dir ${HDF5_INCLUDE_DIRS} )
    message( STATUS "\t# INCLUDE          : " ${_dir} )
  endforeach()
endif()
if ( AgMD2_FOUND )
  message( STATUS "# AgMD2  INCLUDE_DIR    : "  ${AgMD2_INCLUDE_DIR} )
  if ( WIN32 )
    get_target_property( _value ${AgMD2_LIBRARIES} IMPORTED_LOCATION )
    message( STATUS "# AgMD2  LIBRARIES      : "  ${_value} )
    get_target_property( _value ${AgMD2_LIBRARIES} IMPORTED_IMPLIB )
    message( STATUS "                        : "  ${_value} )
  else()
    message( STATUS "# AgMD2  LIBRARIES      : "  ${AgMD2_LIBRARIES} )
  endif()
endif()
message( STATUS "# AqDrv4 INCLUDE_DIR    : "  ${AqDrv4_INCLUDE_DIR} )
if ( AqDrv4_FOUND )
  if ( WIN32 )
    get_target_property( _value ${AqDrv4_LIBRARIES} IMPORTED_LOCATION )
    message( STATUS "# AqDrv4 LIBRARIES      : "  ${_value} )
    get_target_property( _value ${AqDrv4_LIBRARIES} IMPORTED_IMPLIB )
    message( STATUS "                        : "  ${_value} )
  else()
    message( STATUS "# AqDrv4 LIBRARIES      : "  ${AqDrv4_LIBRARIES} )    
  endif()
endif()
message( STATUS "# TARGET_ARCHITECTURE   : "  ${__arch} )
if ( CROSS_COMPILING )
  message( STATUS "# PACKAGE_ARCHITECTURE  : "  ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE} )
endif()
message( STATUS "# CMAKE_BUILD_TYPE      : "  ${CMAKE_BUILD_TYPE} )
message( STATUS "# CMAKE_GENERATOR       : "  ${CMAKE_GENERATOR} )
message( STATUS "# VERSION               : "  ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )
message( STATUS "---------- END SUMMARY ------------" )
