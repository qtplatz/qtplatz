
add_subdirectory( libs )

if ( QT_FOUND )
  add_subdirectory( plugins )
endif()

if ( NOT APPLE )
  if ( AqMD3_FOUND )
    add_subdirectory( diag-aqmd3 )
  endif()
endif()
