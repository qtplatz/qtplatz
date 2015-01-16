#
project (adtoolkit)

cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )

list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules )

# qtplatz.cmake cofigure boost & compiler
include (qtplatz.cmake)

set(QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR} )
set(QTPLATZ_BINARY_DIR ${QTPLATZ_SOURCE_DIR} )

#
# Output directories layout, which is equivalent to original QTC layout
#
set(CMAKE_INSTALL_PREFIX ${QTPLATZ_SOURCE_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/lib) # static libs, dll's import lib/exp

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/MS-Cheminformatics ) #plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/MS-Cheminformatics ) #plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/MS-Cheminformatics ) #plugins

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${QTPLATZ_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_BINARY_DIR}/bin )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_BINARY_DIR}/bin )

set( ACE_ROOT $ENV{ACE_ROOT} )

find_package( Boost REQUIRED )
find_package( Qt5 REQUIRED PrintSupport Svg Widgets Gui Core )
find_package( qwt )
find_package( rdkit )

include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

if ( NOT WIN32 )
  if ( Linux )
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
  endif()
endif()

add_definitions(-DUNICODE -D_UNICODE)

if (MSVC)
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
add_subdirectory (contrib)
