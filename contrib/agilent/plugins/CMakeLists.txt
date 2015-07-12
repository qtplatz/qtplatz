
if ( NOT QTPLATZ_CORELIB_ONLY )

  if ( WIN32 )
    add_subdirectory( u5303aplugin )
  endif()
  add_subdirectory( ap240plugin )

endif()