
add_subdirectory( libs )
add_subdirectory( plugins )

if ( AgMD2_FOUND )
  add_subdirectory( diag-u5303a )
endif()
