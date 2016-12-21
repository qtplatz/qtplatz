
if ( WITH_AqDrv4 AND AqDrv4_FOUND )
  add_subdirectory( ap240plugin )
endif()

if ( WITH_AgMD2 AND AgMD2_FOUND )
  add_subdirectory( u5303aplugin )
endif()
