cmake_minimum_required (VERSION 2.8.9)

add_subdirectory( lrpfile )

if ( Qt5_FOUND )
  add_subdirectory( shrader_lrpfile )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND Qt5_FOUND )
  add_subdirectory( lrpviewer )
endif()
