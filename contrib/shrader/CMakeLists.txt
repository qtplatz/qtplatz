
add_subdirectory( lrpfile )

if ( QT_FOUND )
  add_subdirectory( shrader_lrpfile )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QT_FOUND AND QWT_FOUND )
  add_subdirectory( lrpviewer )
endif()
