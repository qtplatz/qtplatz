
add_subdirectory( libs )

if ( Qt5_FOUND )
#  add_subdirectory( plugins )
endif()

if ( NOT WIN32 AND NOT APPLE )
  if ( AqMD3_FOUND )
    add_subdirectory( diag-aqmd3 )
  endif()
endif()
