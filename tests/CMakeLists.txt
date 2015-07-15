cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( tests )

set( QTPLATZ_SORUCE_DIR ${CMAKE_SOURCE_DIR}/.. )
set( QTPLATZ_BINARY_DIR ${CMAKE_BINARY_DIR}/.. )

find_path( qtplatz_config_DIR NAMES "qtplatz-config.cmake"  HINTS ${QTPLATZ_BINARY_DIR} )

if ( NOT qtplatz_config_DIR )
  message( FATAL_ERROR "qtplatz-config.cmake not find." )
endif()
include ( ${qtplatz_config_DIR}/qtplatz-config.cmake )

add_subdirectory( filename_splitter )
add_subdirectory( formula_parser )
add_subdirectory( isotope )
