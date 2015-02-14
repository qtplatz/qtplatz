#
project (qtplatz)

cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )

set(QMAKE_LAYOUT CACHE BOOL 0)

set(QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR} )

if (QMAKE_LAYOUT)
  set(QTPLATZ_BINARY_DIR ${CMAKE_SOURCE_DIR} )  # qmake compatible
else()
  set(QTPLATZ_BINARY_DIR ${CMAKE_BINARY_DIR} )
endif()

# cofigure boost & compiler
include (qtplatz.cmake)
include (qtplatz-version)

#
# Output directories layout, which is equivalent to original QTC layout
#
if (WIN32)
  set(CMAKE_INSTALL_PREFIX "C:/qtplatz" )
elseif(APPLE)
  set(CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
else()
  set(CMAKE_INSTALL_PREFIX "/usr/local/qtplatz" )
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib) # static libs, dll's import lib/exp

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/MS-Cheminformatics ) #plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/MS-Cheminformatics ) #plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/MS-Cheminformatics ) #plugins

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin )

configure_file(
  ${QTPLATZ_SOURCE_DIR}/qtplatz-config.cmake.in
  ${QTPLATZ_BINARY_DIR}/qtplatz-config.cmake @ONLY )

install( FILES
  ${QTPLATZ_BINARY_DIR}/qtplatz-config.cmake
  DESTINATION ${CMAKE_SOURCE_DIR}/lib )

set( ACE_ROOT $ENV{ACE_ROOT} )

find_package( Boost REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

if ( NOT QTPLATZ_CORELIB_ONLY )

  find_package( Qt5 REQUIRED PrintSupport Svg Widgets Gui Core Sql )
  if ( Qt5-NOTFIND )
    message( STATUS "Check PATH environment variable" )
  endif()
  find_package( qwt )
  find_package( rdkit )

endif()

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

if ( NOT WIN32 )
  if ( Linux )
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
  endif()
endif()

add_definitions(-DUNICODE -D_UNICODE)

if (MSVC)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS )
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has native C++11 support.")
else()
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()
endif()

set(CMAKE_DEBUG_POSTFIX "d")

add_subdirectory (src)

if ( NOT QTPLATZ_CORELIB_ONLY )

  add_subdirectory (contrib)
  add_subdirectory (share)

  include( qtplatz-cpack-config )

  foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS} ${Qt5Sql_PLUGINS} )
    get_target_property( _loc ${plugin} LOCATION )
    file( RELATIVE_PATH _rname $ENV{QTDIR}/plugins ${_loc} )
    get_filename_component(_rpath ${_rname} DIRECTORY )
    install( FILES ${_loc} DESTINATION plugins/${_rpath} )
  endforeach()

  include( CPack )

endif()


