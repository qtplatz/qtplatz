
if ( WITH_AP240 )
  if ( AqDrv4_LIBRARY )
    add_subdirectory( ap240plugin )
  endif()
endif()

if ( WITH_AgMD2 )
  if ( AqMD2_INCLUDE_DIR )
    add_subdirectory( u5303aplugin )
  endif()
endif()
