
#find_package( AqDrv4 QUIET )
#find_package( AgMD2 QUIET )

add_subdirectory( libs )
add_subdirectory( plugins )

if ( AgMD2_INCLUDE_DIR )
  add_subdirectory( diag-u5303a )
endif()
