
if ( NOT QTPLATZ_CORELIB_ONLY )

  if ( WIN32 )
    add_subdirectory( u5303aplugin )
  endif()

  if ( WIN32 OR ${CMAKE_SYSTEM} MATCHES "Linux" )
    add_subdirectory( ap240plugin )
  endif()

endif()