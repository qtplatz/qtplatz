cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( tests )

set ( QTPLATZ_BUILD_DIR ${QTPLATZ_BUILD_DIR} CACHE STRING "destination directory" )

find_path( qtplatz_config_DIR NAMES "qtplatz-config.cmake" PATHS
  "${QTPLATZ_BUILD_DIR}"
  "${CMAKE_SOURCE_DIR}/../../build-Linux-x86_64/qtplatz.debug" )

if ( NOT qtplatz_config_DIR )
  message( FATAL_ERROR "qtplatz-config.cmake not find." )
endif()
include ( ${qtplatz_config_DIR}/qtplatz-config.cmake )

#add_subdirectory( filename_splitter )
add_subdirectory( formula_parser )
add_subdirectory( test_ptree )
#add_subdirectory( isotope )
#add_subdirectory( test_adfs )

find_package( OpenCV 3 QUIET )
find_package( ArrayFire QUIET )

if ( OpenCV_FOUND AND ArrayFire_FOUND )
  add_subdirectory( test_vision )
endif()
