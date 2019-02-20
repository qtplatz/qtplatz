#
cmake_minimum_required ( VERSION 3.7 )  # for GREATER_EQUAL
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )
cmake_policy( SET CMP0053 NEW )

project (qtplatz)

if (NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
  set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set ( VERBOSE OFF )
set ( CMAKE_ECLIPSE_VERSION 4.5 )
set ( DEBUG_SYMBOL OFF CACHE BOOL "DEBUG_SYMBOL" )

set ( QTPLATZ_CORELIB_ONLY OFF CACHE BOOL "QTPLATZ CORE LIBRARY ONLY" )

option( WITH_RDKIT         "Include chemistry support"  ON )
option( WITH_OPENCV        "Include OpenCV"             ON )
option( WITH_QT5           "Include QT5"                ON )
option( WITH_QWT           "Include QWT"                ON )
option( WITH_AqDrv4        "Keysight AP240/DC122"       ON )
option( WITH_AgMD2         "Keysight U5303A"            ON )

if ( CMAKE_CROSSCOMPILING )
  option( WITH_CUDA          "CUDA"                     OFF )
else()
  option( WITH_CUDA          "CUDA"                     ON )
endif()
option( WITH_ARRAYFIRE     "ArrayFire"                  OFF )

set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR} )
set( QTPLATZ_BINARY_DIR ${CMAKE_BINARY_DIR} )

if ( WITH_QT5 )
  if ( QTDIR STREQUAL "" AND DEFINED $ENV{QTDIR} )
    set( QTDIR $ENV{QTDIR} )
  endif()
endif()

include (qtplatz.cmake)
include (version)

if ( WIN32 )
  set( CMAKE_INSTALL_PREFIX "C:/QtPlatz" )
elseif( APPLE )
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
else()
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
endif()

if ( APPLE )

  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib) # static libs, dll's import lib/exp
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/Frameworks )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/MacOS )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/MacOS )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/MacOS )
  set( CMAKE_SHARED_MODULE_SUFFIX ".dylib" )
  set( CMAKE_MACOSX_RPATH 1 )
  set( CMAKE_SKIP_BUILD_RPATH  FALSE )
  set( QTPLATZ_PLUGIN_DIRECTORY ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/PlugIns )
  set( QTPLATZ_PLUGIN_DESTINATION bin/qtplatz.app/Contents/PlugIns )

else()

  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib ) # static libs, dll's import lib/exp
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/lib/qtplatz ) #plugins
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin )  # .exe | .dll
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin )

  set( QTPLATZ_PLUGIN_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins )
  set( QTPLATZ_PLUGIN_DESTINATION lib/qtplatz/plugins )

  set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/qtplatz:${CMAKE_INSTALL_PREFIX}/lib" )
  set( CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE )

  if ( CMAKE_CROSSCOMPILING AND WITH_QT5 )
    set( CMAKE_PREFIX_PATH /usr/local/qt5 )
  endif()

endif()

if( WIN32 )
  set( CPACK_SET_DESTDIR OFF )  # WiX does not support this, and raise an error
  set( CPACK_WIX_UPGRADE_GUID "F51A5CFA-AB54-4FAE-BBDC-32C42F4C039A" )
  set( CPACK_WIX_PROGRAM_MENU_FOLDER "QtPlatz Mass Spectrometry" )
  set( CPACK_WIX_CMAKE_PACKAGE_REGISTRY "QtPlatz" ) # HLM\Software\Kitware\CMake\Packages\QtPlatz
  set( CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_INSTALL_PREFIX}" )
  set( CPACK_WIX_SKIP_PROGRAM_FOLDER ON )
  set( QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY "bin" )
  set( QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY "bin" )
elseif ( APPLE )
  set( CPACK_SET_DESTDIR ON )
  set( QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY "lib/qtplatz" )
  set( QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY "lib/qtplatz" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.12" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.12" )
else() # Linux
  set( CPACK_SET_DESTDIR ON )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fext-numeric-literals -Wno-psabi" )
  set( QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY "lib" )
  set( QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY "lib/qtplatz" )
endif()

if ( WIN32 )
  set(CMAKE_DEBUG_POSTFIX "d")
endif()
if ( APPLE )
  set(CMAKE_DEBUG_POSTFIX "_debug")
endif()

# adplugin now uses boost_dll, which require 1.61
find_package( Boost 1.62 REQUIRED COMPONENTS
  atomic
  bzip2
  chrono
  date_time
  filesystem
  iostreams
#  locale
  log
  program_options
  regex
  serialization
  system
  thread
  timer
  wserialization
  )

if ( Boost_FOUND )
  include_directories( ${Boost_INCLUDE_DIRS} )
endif()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

if ( WITH_QWT )
  find_package( qwt )
  if ( NOT QWT_LIBRARIES )
    message( FATAL_ERROR "No QWT library found" )
  endif()
endif()

find_package( Eigen3 REQUIRED )

if ( WITH_RDKIT )
  find_package( rdkit QUIET )
  if ( rdkit_FOUND )
    add_definitions( -DHAVE_RDKit=1 )
    get_filename_component ( RDBASE ${RDKit_LIBRARY_DIRS} DIRECTORY )
  else()
    message( STATUS "No RDKit found" )
  endif()
endif()

if ( WITH_OPENCV )
  if ( APPLE )
    list ( APPEND CMAKE_PREFIX_PATH "/usr/local/opt/opencv3/share/OpenCV")
  endif()
  if ( WIN32 )
    list ( APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/../opencv.release/install "C:/OpenCV")
    add_definitions( -DNOMINMAX ) # supress max/min macro in windef.h
  endif()
  find_package( OpenCV 3 QUIET )
  if ( OpenCV_FOUND )
    find_package( OpenCV 3 REQUIRED core imgproc video highgui features2d )
    add_definitions( -DHAVE_OPENCV=1 )
  else()
    find_package( OpenCV 4 QUIET )
    if ( OpenCV_FOUND )
      find_package( OpenCV 4 REQUIRED core imgproc video highgui features2d )
      add_definitions( -DHAVE_OPENCV=1 )
    endif()
  endif()

  find_package( HDF5 QUIET )
  if ( HDF5_FOUND )
    add_definitions( -DHAVE_HDF5=1 )
  endif()

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

if ( WITH_CUDA )
  find_package( CUDA QUIET )
  if ( CUDA_FOUND )
    add_definitions( -DHAVE_CUDA=1 )
  endif()
endif()

if ( WITH_ARRAYFIRE )
  find_package( ArrayFire )
  if ( ArrayFire_FOUND )
    add_definitions( -DHAVE_ARRAYFIRE=1 )
  endif()
endif()

if ( WIN32 )
  set( SOLUTION_FILE "qtplatz" )
  configure_file(
    ${QTPLATZ_SOURCE_DIR}/makefile.nmake.in
    ${CMAKE_BINARY_DIR}/makefile @ONLY
    )

  file( WRITE "${CMAKE_BINARY_DIR}/build-package.bat" "msbuild /m /p:Configuration=Release PACKAGE.vcxproj" )

  if ( DEBUG_SYMBOL )
    set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DEBUG" )
    set( CMAKE_MODULE_LINKER_FLAGS_RELEASE "/DEBUG" )
    set( CMAKE_CXX_FLAGS_RELEASE "/MD /Zi /O2 /Ob2 /D NDEBUG" )
  endif()

endif()

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

add_definitions(-DUNICODE -D_UNICODE)
add_subdirectory ( src )
add_subdirectory ( contrib )
if ( Qt5_FOUND )
  add_subdirectory ( share )
endif()
add_subdirectory( tests )

########### CPack ########
#message( STATUS "------------ cpack ----------------" )

set( CPACK_PACKAGE_NAME "qtplatz" )

include( qtplatz-cpack-config )

if ( WIN32 )

  set( CPACK_GENERATOR "WIX" )
  include( msvc-cpack )
  include( rdkit-cpack )
  include( boost-cpack )
  include( qt5-cpack )
  if ( OpenCV_FOUND )
    include( opencv-cpack )
  endif()

elseif( APPLE )

  SET( MACOSX_BUNDLE_STARTUP_COMMAND qtplatz)
  set( CPACK_BINARY_DRAGNDROP ON )

  include( boost-cpack )
  include( qt5-cpack )
  include( rdkit-cpack )

elseif ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  set( CPACK_DEB_COMPONENT_INSTALL 1 )
  set( CPACK_GENERATOR "DEB" )
  set( CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst;${CMAKE_CURRENT_BINARY_DIR}/prerm" )
  set( CPACK_DEBIAN_PACKAGE_DESCRIPTION "QtPlatz -- Mass spectrometry data system desktop application" )

  if ( RTC_ARCH_ARM )
    set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
  endif()

  set( INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib )
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

message( STATUS "======== QMAKE; " ${QMAKE} )

include (config_qtplatz_config)

config_qtplatz_config( "TRUE" )   # installed
config_qtplatz_config( "FALSE" )  # in source tree
install( FILES ${CMAKE_BINARY_DIR}/qtplatz-config_installed.cmake DESTINATION "." RENAME "qtplatz-config.cmake" COMPONENT headers )

configure_file(
  ${QTPLATZ_SOURCE_DIR}/doxyfile.in
  ${QTPLATZ_BINARY_DIR}/Doxyfile )

include( CPack )

message( STATUS "---------- SUMMARY ------------" )
message( STATUS "# QTPLATZ_BINARY_DIR    : " ${QTPLATZ_BINARY_DIR} )
message( STATUS "# QTPLATZ_CORELIB_ONLY  : " ${QTPLATZ_CORELIB_ONLY} )
message( STATUS "# DEBUG SYMBOL          : " ${DEBUG_SYMBOL} )
message( STATUS "# COMPILER VERSION      : " ${CMAKE_CXX_COMPILER_VERSION} )
message( STATUS "# Boost_VERSION         : " ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
message( STATUS "# Boost_INCLUDE_DIRS    : " ${Boost_INCLUDE_DIRS} )
message( STATUS "# Boost_LIBRARY_DIRS    : " ${Boost_LIBRARY_DIRS} )
if ( WITH_QWT )
  get_target_property( _value ${QWT_LIBRARIES} IMPORTED_LOCATION )
  message( STATUS "# QWT                   : ${_value};${QWT_INCLUDE_DIR}" )
endif()
message( STATUS "# XMLPATTERNS           : " ${XMLPATTERNS} )
if ( libxml2_FOUND )
  message( STATUS "# libxml2               : ${libxml2_LIBRARIES};${libxml2_INCLUDE_DIRS}" )
  message( STATUS "# libxslt               : ${libxslt_LIBRARIES};${libxslt_INCLUDE_DIRS}" )
endif()
if ( Eigen3_FOUND )
  message( STATUS "# Eigen3                : " ${EIGEN3_INCLUDE_DIR} )
endif()
message( STATUS "# RDKit                 : " ${rdkit_FOUND} )
if ( rdkit_FOUND )
  message( STATUS "# RDKit VERSION         : " ${RDKit_PACKAGE_VERSION} )
  foreach ( _dir ${RDKit_LIBRARY_DIRS} )
    message( STATUS "\t# LIBRARY          : " ${_dir} )
  endforeach()
  #if ( WIN32 )
    message( STATUS "\t# RDBASE           : " ${RDBASE} )
    if ( VERBOSE )
      foreach ( _dir ${RDKit_INCLUDE_DIRS} )
	message( STATUS "\t# INCLUDE          : " ${_dir} )
      endforeach()
      foreach( _name ${RDKit_LIBRARIES} )
	get_target_property( _value ${_name} IMPORTED_LOCATION_RELEASE )
	message( STATUS "\t\t " ${_name} "\t" ${_value} )
      endforeach()
    endif()
  #endif()
endif()
if ( WITH_CUDA )
  if ( CUDA_FOUND )
    message( STATUS "# CUDA VERSION          : "  ${CUDA_VERSION} )
    message( STATUS "\t# CUDA Libraries   : ${CUDA_CUDA_LIBRARY};${CUDA_CUDART_LIBRARY}")
    message( STATUS "\t# CUDA Include     : ${CUDA_INCLUDE_DIRS}")
  else()
    message( STATUS "# CUDA VERSION          : NOT FOUND" )
  endif()
endif()

if ( WITH_ARRAYFIRE AND ArrayFire_FOUND )
  message( STATUS "# ArrayFire VERSION     : "  ${ArrayFire_VERSION} )
  message( STATUS "\t# ArrayFire_CPU    : "  ${ArrayFire_CPU_FOUND} )
  message( STATUS "\t# ArrayFire_CUDA   : "  ${ArrayFire_CUDA_FOUND} )
  message( STATUS "\t# ArrayFire_OpenCL : "  ${ArrayFire_OpenCL_FOUND} )
  message( STATUS "\t# ArrayFire_Unified: "  ${ArrayFire_Unified_FOUND} )
  foreach( _dir ${ArrayFire_INCLUDE_DIRS} )
    message( STATUS "\t# INCLUDE          : "   ${_dir} )
  endforeach()
  set ( aflib )
  foreach( _lib ${ArrayFire_LIBRARIES} ${ArrayFire_CPU_LIBRARIES}
      ${ArrayFire_CUDA_LIBRARIES} ${ArrayFire_OpenCL_LIBRARIES} ${ArrayFire_Unified_LIBRARIES} )
    list( APPEND aflib ${_lib} )
  endforeach()
  list( REMOVE_DUPLICATES aflib )
  message( STATUS "\t# LIBRARIES        : ${aflib}" )
endif()

message( STATUS "# OpenCV VERSION        : "  ${OpenCV_VERSION} )
if ( OpenCV_FOUND )
  message( STATUS "\t# INCLUDE          : ${OpenCV_INCLUDE_DIRS}" )
endif()
message( STATUS "# HDF5 VERSION          : " ${HDF5_VERSION} )
if ( HDF5_FOUND )
  message( STATUS "\t# INCLUDE          : ${HDF5_INCLUDE_DIRS}" )
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
if ( Boost_FOUND AND VERBOSE )
  message( STATUS "# Boost_LIBRARIES" )
  foreach( _lib ${Boost_LIBRARIES} )
    message( STATUS "\t" ${_lib} )
  endforeach()
endif()
message( STATUS "# TARGET_ARCHITECTURE   : "  ${__arch} )
if ( CROSS_COMPILING )
  message( STATUS "# PACKAGE_ARCHITECTURE  : "  ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE} )
endif()
message( STATUS "# CMAKE_BUILD_TYPE      : "  ${CMAKE_BUILD_TYPE} )
message( STATUS "# CMAKE_GENERATOR       : "  ${CMAKE_GENERATOR} )
message( STATUS "# VERSION               : "  ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )
message( STATUS "# CMAKE_INSTALL_PREFIX  : "  ${CMAKE_INSTALL_PREFIX} )
message( STATUS "# CPACK_PACKAGE_INSTALL_DIRECTORY: " ${CPACK_PACKAGE_INSTALL_DIRECTORY} )
message( STATUS "# Boost_LIBRARY_DIRS    : "  ${Boost_LIBRARY_DIRS} )
message( STATUS "# Boost_Use_STATIC_LIBS : "  ${Boost_USE_STATIC_LIBS} )
if ( Boost_FOUND AND VERBOSE )
  foreach( name ${Boost_LIBRARIES} )
    message( STATUS "\t\t " ${name} )
  endforeach()
endif()
message( STATUS "# CMAKE_CXX_STANDARD    : " ${CMAKE_CXX_STANDARD} )
message( STATUS "# Qt5_DIR               : " ${Qt5_DIR} "\tQt5_FOUND=" ${Qt5_FOUND} )
message( STATUS "# QTDIR                 : " ${QTDIR} )
message( STATUS "# QMAKE                 : " ${QMAKE} )
if ( WITH_QT5 AND DEFINED $ENV{QT_PLUGIN_PATH} )
  message( STATUS "################## WARNING ###################" )
  message( STATUS "QT_PLUGIN_PATH has set to $ENV{QT_PLUGIN_PATH}" )
  message( STATUS "################## WARNING ###################" )
endif()
message( STATUS "---------- END SUMMARY ------------" )
