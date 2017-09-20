
add_subdirectory( libs )
if ( Qt5_FOUND )
  add_subdirectory( plugins )
endif()

if ( AgMD2_FOUND )
  add_subdirectory( diag-u5303a )
endif()
