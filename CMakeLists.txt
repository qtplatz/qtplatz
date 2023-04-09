#
if ( WIN32 )
  cmake_minimum_required ( VERSION 3.20.1 )
else()
  cmake_minimum_required ( VERSION 3.18.2 )  # for GREATER_EQUAL
endif()

cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW ) # MAC OSX RPATH
cmake_policy( SET CMP0053 NEW )

project (qtplatz)

if ( NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS )
  set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set ( VERBOSE OFF )
set ( CMAKE_ECLIPSE_VERSION 4.5 )
set ( DEBUG_SYMBOL OFF CACHE BOOL "DEBUG_SYMBOL" )

set ( QTPLATZ_CORELIB_ONLY OFF CACHE BOOL "QTPLATZ CORE LIBRARY ONLY" )
set ( Boost_NO_BOOST_CMAKE OFF )

if ( APPLE )
#  add_definitions( "-Wno-unused-local-typedefs" )
#  add_definitions( -Wno-deprecated-declarations )
  # -- uncomment following line, is you would like to build a universal binary --
  # set( CMAKE_OSX_ARCHITECTURES "arm64;x86_64" )
  #
  # set( CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version") # Catalina
endif()

if ( CMAKE_COMPILER_IS_GNUCC )
#  add_definitions( "-Wno-deprecated-declarations" )
endif()

option( WITH_RDKIT         "Include chemistry support"  ON )
option( WITH_OPENCV        "Include OpenCV"             ON )
option( WITH_QT5           "Include QT5"                ON )
option( WITH_QWT           "Include QWT"                ON )
option( WITH_AqDrv4        "Keysight AP240/DC122"       OFF )
option( WITH_AgMD2         "Keysight U5303A"            ON )
option( WITH_AqMD3         "Keysight U5303A/SA220"      ON )
if ( APPLE )
  option( WITH_Wolfram     "Wolfram Engine support"     OFF )
else()
  option( WITH_Wolfram     "Wolfram Engine support"     OFF )
endif()
option( WITH_TESTS         "Tests"                      OFF )
if ( CMAKE_CROSSCOMPILING )
  option( WITH_CUDA        "CUDA"                       OFF )
  option( WITH_PYTHON3     "Python3 modules"            OFF )
else()
  option( WITH_CUDA        "CUDA"                       ON )
  if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "6.3.0" )
    option( WITH_PYTHON3     "Python3 modules"            OFF )
  else()
    option( WITH_PYTHON3     "Python3 modules"            ON )
  endif()
endif()

set( CMAKE_WARN_DEPRECATED TRUE )
set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR} )
set( QTPLATZ_BINARY_DIR ${CMAKE_BINARY_DIR} )

if ( WITH_QT5 )
  find_package( QT NAMES Qt6 Qt5 OPTIONAL COMPONENTS Core )
  message( STATUS "############# QT_FOUND : ${QT_FOUND} -- ${QT_VERSION} ###############" )
  if ( QT_FOUND )
    add_definitions( "-DQT_MESSAGELOGCONTEXT" ) # <-- enable file, line in qDebug output
    find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core PrintSupport Svg Sql Widgets Gui OpenGL Multimedia )
    find_program( XMLPATTERNS NAMES xmlpatterns HINTS "${QTDIR}/bin" "/opt/Qt/5.15.2/bin" )
    if ( NOT XMLPATTERNS )
      message( FATAL_ERROR "xmlpatterns command not found" )
    endif()
    if ( QT_VERSION_MAJOR LESS_EQUAL 5 )
      set ( IDE_VERSION "3.2.81" ) # defined in src/app/app_version.h
      set ( IDE_PLUGIN_PATH "lib/qtplatz/plugins" ) # <-- for qtplatz 5
      set ( QTC_BINARY_DIR  "" )  # <-- same as CMAKE_BINARY_DIR
    endif()
  endif()

endif()

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules" )
if ( WIN32 )
  list ( APPEND CMAKE_MODULE_PATH "C:/Eigen3/share/eigen3/cmake" )
endif()

include (qtplatz.cmake)
include (version)
include (boost.cmake)
include (qtplatz_api)

if ( WIN32 )
  set( CMAKE_INSTALL_PREFIX "C:/QtPlatz" )
elseif( APPLE )
  set( CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/package/bin" )
else()
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
endif()

if ( APPLE )

  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib) # static libs, dll's import lib/exp
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/Frameworks )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin ) #/qtplatz.app/Contents/MacOS )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin ) #/qtplatz.app/Contents/MacOS )
  set( CMAKE_SHARED_MODULE_SUFFIX             ".dylib" )
  set( CMAKE_MACOSX_RPATH                     1 )
  set( CMAKE_SKIP_BUILD_RPATH                 FALSE )
  set( CMAKE_BUILD_WITH_INSTALL_RPATH         FALSE )
  set( QTPLATZ_PLUGIN_DIRECTORY               ${QTPLATZ_BINARY_DIR}/bin/qtplatz.app/Contents/PlugIns )
  set( QTPLATZ_PLUGIN_DESTINATION             bin/qtplatz.app/Contents/PlugIns )

else()

  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib ) # static libs, dll's import lib/exp
  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib ) # static libs, dll's import lib/exp
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/lib/qtplatz ) #plugins
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin )  # .exe | .dll
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin )

  set( QTPLATZ_PLUGIN_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins )
  set( QTPLATZ_PLUGIN_DESTINATION lib/qtplatz/plugins )

  set( CMAKE_SKIP_BUILD_RPATH  FALSE )
  set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
  set( CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE )
  set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/qtplatz:${CMAKE_INSTALL_PREFIX}/lib" )

  if ( CMAKE_CROSSCOMPILING AND WITH_QT5 )
    set( CMAKE_PREFIX_PATH /usr/local/qt5 )
  endif()

endif()

if( WIN32 )
  set( CPACK_SET_DESTDIR OFF )  # WiX does not support this, and raise an error
  set( CPACK_WIX_UPGRADE_GUID                 "F51A5CFA-AB54-4FAE-BBDC-32C42F4C039A" )
  set( CPACK_WIX_PROGRAM_MENU_FOLDER          "QtPlatz Mass Spectrometry" )
  set( CPACK_WIX_CMAKE_PACKAGE_REGISTRY       "QtPlatz" ) # HLM\Software\Kitware\CMake\Packages\QtPlatz
  set( CPACK_PACKAGE_INSTALL_DIRECTORY        "${CMAKE_INSTALL_PREFIX}" )
  set( CPACK_WIX_SKIP_PROGRAM_FOLDER          ON )
  set( QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY    "bin" )
  set( QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY "bin" )
elseif ( APPLE )
  set( CPACK_SET_DESTDIR ON )
  set( QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY    "lib/qtplatz" )
  set( QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY "lib/qtplatz" )
  #  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.14" )
  #  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.14" )
else() # Linux
  set( CPACK_SET_DESTDIR ON )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fext-numeric-literals -Wno-psabi" )
  set( QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY    "lib" )
  set( QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY "lib/qtplatz" )
endif()

if ( WITH_PYTHON3 )
  find_package( Python3 COMPONENTS Interpreter Development )

  if ( Python3_FOUND )
    if ( WIN32 )
      # workaround: boost_pythonXXYY is the canonical name on windows
      set ( python_libs python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR} )
    elseif( NOT APPLE )
      set ( python_libs python )
    endif()
  endif()
endif()

if ( WIN32 )
  set(CMAKE_DEBUG_POSTFIX "d")
  set( boost_libs "bzip2" )
endif()

if ( APPLE )
  set(CMAKE_DEBUG_POSTFIX "_debug")
else()
endif()


##############################################
### find rdkit will include rdkit-config.cmake that removes Boost_LIBRARIES,
### so find rdkit must be invoked before find boost
##############################################

if ( WITH_RDKIT )
  find_package( rdkit QUIET )
  if ( rdkit_FOUND )
    add_definitions( -DHAVE_RDKit=1 )
    get_filename_component ( RDBASE ${RDKit_LIBRARY_DIRS} DIRECTORY )
    find_package( Eigen3 REQUIRED )
  else()
    add_definitions( -DHAVE_RDKit=0 )
    message( STATUS "No RDKit found" )
  endif()
endif()

##############################################
### find boost
##############################################
## boost-1.75 required boost::json, which uses in libadcv and libvideo
set( Boost_NO_WARN_NEW_VERSIONS ON )

find_package( Boost 1.75 REQUIRED COMPONENTS
  atomic
  ${boost_libs} # windows
  chrono
  container
  date_time
  filesystem
  iostreams
  json
  log
  program_options
  regex
  random
  serialization
  system
  thread
  timer
  wserialization
  OPTIONAL_COMPONENTS ${python_libs}
  )

if ( Boost_FOUND )
  include_directories( ${Boost_INCLUDE_DIRS} )
  if ( WIN32 )
    add_definitions( -DBOOST_UUID_FORCE_AUTO_LINK=1 )
  endif()
  add_definitions( -DBOOST_BIND_GLOBAL_PLACEHOLDERS=1 )
  add_definitions( -DBOOST_ALLOW_DEPRECATED_HEADERS=1 )
else()
  message( FATAL_ERROR "#################### " ${Boost_FOUND} )
endif()

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/libs"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/xmlparser/pugixml-1.11/src"
  )

if ( WITH_QWT )
  find_package( qwt )
  if ( NOT QWT_LIBRARIES )
    message( FATAL_ERROR "No QWT library found" )
  endif()
  if ( ${QWT_VERSION} VERSION_LESS "6.2.0" )
    message( FATAL_ERROR "QWT VERSION MULST BE 6.2.0 OR HIGHER" )
  endif()
endif()

if ( WITH_OPENCV )
  if ( APPLE )
    list ( APPEND CMAKE_PREFIX_PATH "/usr/local/opt/opencv3/share/OpenCV")
  endif()
  if ( WIN32 )
    find_file( OPENCV_ROOT "OpenCV" HINTS "C:/" "C:/opt" )
    if ( OPENCV_ROOT )
      list ( APPEND CMAKE_PREFIX_PATH ${OPENCV_ROOT} )
    else()
      list ( APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/windows/opencv.build/install )
    endif()
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
else()
  find_path( AqDrv4_INCLUDE_DIR NAMES AcqirisImport.h PATHS ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
endif()

if ( WITH_AqMD3 )
  find_package( AqMD3 QUIET )
  if ( AqMD3_FOUND )
    add_definitions( -DHAVE_AqMD3=1 )
  endif()
endif()

if ( NOT AqMD3_FOUND AND WITH_AgMD2 )
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

if ( WITH_Wolfram )
  find_package( Wolfram )
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
else()

  if ( DEBUG_SYMBOL )
    set( CMAKE_CXX_FLAGS_RELEASE "-g" )
  endif()

endif()

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

add_definitions(-DUNICODE -D_UNICODE)
add_subdirectory ( src )
if ( ${QT_VERSION_MAJOR} EQUAL 5 )
  add_subdirectory ( contrib )
endif()
if ( Qt5_FOUND )
  add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
  add_subdirectory ( share )
endif()

if ( WITH_TESTS )
  add_subdirectory( tests )
endif()

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

  set( CPACK_PACKAGE_FILE_NAME qtplatz )
  SET( MACOSX_BUNDLE_STARTUP_COMMAND qtplatz)
  set( CPACK_BINARY_DRAGNDROP ON )
  set( MACOSX_BUNDLE_BUNDLE_NAME qtplatz )
  if ( ${QT_VERSION_MAJOR} EQUAL 5 )
    include( boost-cpack )
    include( qt5-cpack )
    include( rdkit-cpack )
  endif()

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

include (config_qtplatz_config)

config_qtplatz_config( "TRUE" )   # installed
config_qtplatz_config( "FALSE" )  # in source tree
install( FILES ${CMAKE_BINARY_DIR}/qtplatz-config_installed.cmake DESTINATION "." RENAME "qtplatz-config.cmake" COMPONENT headers )

configure_file(
  ${QTPLATZ_SOURCE_DIR}/doxyfile.in
  ${QTPLATZ_BINARY_DIR}/Doxyfile )

include( CPack )

message( STATUS "---------- SUMMARY ------------" )
message( STATUS "# COMPILER VERSION      : " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION} )
message( STATUS "# QTPLATZ_BINARY_DIR    : " ${QTPLATZ_BINARY_DIR} )
if ( QTPLATZ_CORELIB_ONLY )
  message( STATUS "# QTPLATZ_CORELIB_ONLY  : " ${QTPLATZ_CORELIB_ONLY} )
endif()
if ( DEBUG_SYMBOL )
  message( STATUS "# DEBUG SYMBOL          : " ${DEBUG_SYMBOL} )
endif()

message( STATUS "# Boost_VERSION         : " ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
message( STATUS "# BOOST_ROOT            : " ${BOOST_ROOT} )
message( STATUS "# Boost_Use_STATIC_LIBS : " ${Boost_USE_STATIC_LIBS} )
message( STATUS "# Boost_INCLUDE_DIRS    : " ${Boost_INCLUDE_DIRS} )
message( STATUS "# Boost_LIBRARY_DIRS    : " ${Boost_LIBRARY_DIRS} )
if ( Boost_FOUND AND VERBOSE )
  message( STATUS "# Boost_LIBRARIES" )
  foreach( _lib ${Boost_LIBRARIES} )
    if ( ${_lib} STREQUAL "debug" OR ${_lib} STREQUAL "optimized" )
      set( _pfx ${_lib} )
    else()
      message( STATUS "\t" ${_pfx} "\t" "${_lib}" )
    endif()
  endforeach()
endif()

if ( WITH_QWT )
  get_target_property( qwt_release ${QWT_LIBRARIES} IMPORTED_LOCATION )
  get_target_property( qwt_debug ${QWT_LIBRARIES} IMPORTED_LOCATION_DEBUG )
  message( STATUS "# QWT                   : ${qwt_release};${qwt_debug};${QWT_INCLUDE_DIR};${QWT_VERSION_STRING}" )
  message( STATUS "# QWT VERSION           : ${QWT_VERSION}" )
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
  message( STATUS "\t# RDBASE           : " ${RDBASE} )
  if ( VERBOSE )
    foreach ( _dir ${RDKit_INCLUDE_DIRS} )
      message( STATUS "\t# INCLUDE          : " ${_dir} )
    endforeach()
    foreach( _name ${RDKit_LIBRARIES} )
      get_target_property( _value ${_name} IMPORTED_LOCATION_RELEASE )
      message( STATUS "\t\t " ${_name} "\t" ${_value} )
    endforeach()
  else()
    message( STATUS "\t# RDKit_LIBRARY_DIRS: " ${RDKit_LIBRARY_DIRS} )
    message( STATUS "\t# RDKit_INCLUDE_DIRS: " ${RDKit_INCLUDE_DIRS} )
  endif()
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

message( STATUS "# OpenCV VERSION        : "  ${OpenCV_VERSION} )
if ( OpenCV_FOUND )
  message( STATUS "\t# INCLUDE          : ${OpenCV_INCLUDE_DIRS}" )
endif()
message( STATUS "# HDF5 VERSION          : " ${HDF5_VERSION} )
if ( HDF5_FOUND )
  message( STATUS "\t# INCLUDE          : ${HDF5_INCLUDE_DIRS}" )
endif()

if ( AqMD3_FOUND )
  message( STATUS "# AqMD3  INCLUDE_DIR    : "  ${AqMD3_INCLUDE_DIR} )
  foreach( _lib ${AqMD3_LIBRARIES} )
    message( STATUS "# AqMD3  LIBRARIES      : "  "${_lib}" )
    if ( WIN32 )
      get_target_property( _value ${_lib} IMPORTED_LOCATION )
      message( STATUS "\t# AqMD3  LIBRARIES : "  ${_value} )
      get_target_property( _value ${_lib} IMPORTED_IMPLIB )
      message( STATUS "\t                   : "  ${_value} )
    endif()
  endforeach()
else()
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
if ( WITH_Wolfram )
  message( STATUS "# Wolfram               : " "${Wolfram_FOUND}" )
  if ( Wolfram_FOUND )
    message( STATUS "# WSTP_INCLUDE_DIRS     : " "${WSTP_INCLUDE_DIRS}" )
    message( STATUS "# WSTP_LIBRARY_DIRS     : " "${WSTP_LIBRARY_DIRS}" )
    message( STATUS "# WSTP_LIBRARIES        : " "${WSTP_LIBRARIES}" )
  endif()
endif()
message( STATUS     "# Eigen3                : " ${EIGEN3_INCLUDE_DIR} )
if ( WITH_PYTHON3 )
  message( STATUS "# Python3               : " ${Python3_VERSION} )
endif()
message( STATUS "# TARGET_ARCHITECTURE   : "  ${__arch} )
if ( CROSS_COMPILING )
  message( STATUS "# PACKAGE_ARCHITECTURE  : "  ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE} )
endif()
if ( APPLE )
  message(STATUS "# CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
  message(STATUS "# CMAKE_HOST_SYSTEM_PROCESSOR: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
  message(STATUS "# CMAKE_OSX_ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")
endif()
message( STATUS "# CMAKE_BUILD_TYPE      : "  ${CMAKE_BUILD_TYPE} )
message( STATUS "# CMAKE_GENERATOR       : "  ${CMAKE_GENERATOR} )
message( STATUS "# VERSION               : "  ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )
message( STATUS "# CMAKE_INSTALL_PREFIX  : "  ${CMAKE_INSTALL_PREFIX} )
message( STATUS "# CPACK_PACKAGE_INSTALL_DIRECTORY: " ${CPACK_PACKAGE_INSTALL_DIRECTORY} )

message( STATUS "# CMAKE_CXX STD ID VER  : " "${CMAKE_CXX_STANDARD} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message( STATUS "# Qt5_DIR               : " ${Qt5_DIR} )
message( STATUS "# QTDIR                 : " ${QTDIR} )
message( STATUS "# QMAKE                 : " ${QMAKE} )
if ( WITH_QT5 AND DEFINED $ENV{QT_PLUGIN_PATH} )
  message( STATUS "################## WARNING ###################" )
  message( STATUS "QT_PLUGIN_PATH has set to $ENV{QT_PLUGIN_PATH}" )
  message( STATUS "################## WARNING ###################" )
endif()
message( STATUS "---------- END SUMMARY ------------" )

if ( NOT Boost_LIBRARIES )
  message( FATAL_ERROR "Boost_LIBRARIES is empty" )
endif()
