
add_subdirectory( libs )
if ( Qt5_FOUND )
  add_subdirectory( plugins )
endif()

if ( NOT WIN32 AND NOT APPLE )
  if ( AgMD2_FOUND )
    add_subdirectory( diag-u5303a )
  endif()
endif()
