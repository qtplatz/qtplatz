
if ( WITH_AqDrv4 AND AqDrv4_FOUND )
  add_subdirectory( ap240plugin )
endif()

if ( IDE_VERSION VERSION_GREATER_EQUAL "9.0.0" )

  message( STATSU "---- QtCreator 9 found -- not yet ported U5303A plugin module ----" )

else()
  if ( WITH_AgMD2 AND AgMD2_FOUND )
    add_subdirectory( u5303aplugin )
  endif()
endif()
