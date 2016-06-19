
if ( NOT CMAKE_CROSSCOMPILING )

  if ( AqDrv4_INCLUDE_DIR )
    add_subdirectory( ap240 )
    add_subdirectory( ap240controller )
  endif()
  
  if ( AgMD2_INCLUDE_DIR )
    add_subdirectory( u5303a )
  endif()
  
  if ( NOT QTPLATZ_CORELIB_ONLY )
    add_subdirectory( acqrswidgets )  # Qt GUI
  endif()

endif()

add_subdirectory( acqrsinterpreter )
add_subdirectory( acqrscontrols )
