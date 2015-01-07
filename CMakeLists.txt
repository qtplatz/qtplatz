#
cmake_minimum_required (VERSION 2.8.9)

project (adtoolkit)

set(BOOST_VERSION boost-1_57)
set(Boost_USE_STATIC_LIBS ON)

include (qtplatz.cmake)

if (WIN32)
  set( QWT "C:/Qwt-6.1.3-svn" )
endif()

add_subdirectory (src)
add_subdirectory (contrib)

