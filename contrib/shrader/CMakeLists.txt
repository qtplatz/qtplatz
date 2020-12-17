
add_subdirectory( lrpfile )

if ( Qt5_FOUND )
  add_subdirectory( shrader_lrpfile )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND Qt5_FOUND AND QWT_FOUND )
  add_subdirectory( lrpviewer )
endif()
