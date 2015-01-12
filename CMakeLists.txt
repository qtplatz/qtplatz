#
cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
project (adtoolkit)

include (qtplatz.cmake)
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules )
set( Boost_USE_STATIC_LIBS ON )
set( BOOST_VERSION boost-1_57 )
set( BOOST_INCLUDEDIR $ENV{BOOST_ROOT}/include/${BOOST_VERSION} )
set( BOOST_ADDITIONAL_VERSIONS "1.56" "1.57" )

if(WIN32)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
  if ( RTC_ARCH_X64 )
    set( Boost_LIBRARY_DIR "C:/Boost/x86_64/lib")
  endif()
  set(Boost_INCLUDE_DIR "C:/Boost/include/${BOOST_VERSION}")
endif()

find_package( Boost REQUIRED )
find_package( Qt5 REQUIRED PrintSupport Svg Widgets Gui Core )
find_package( qwt )
find_package( rdkitConfig )
if ( rdkit_VERSION )
  message( STATUS "Found RDKit: " ${rdkit_VERSION} " in " ${rdkit_DIR} )
endif()

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

if ( NOT WIN32 )
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
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
