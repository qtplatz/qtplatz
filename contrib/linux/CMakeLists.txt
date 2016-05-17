cmake_minimum_required (VERSION 2.8.9)

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  add_subdirectory( libdgpio )
endif()

