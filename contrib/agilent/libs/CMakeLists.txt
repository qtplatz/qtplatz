
if ( NOT CMAKE_CROSSCOMPILING )
  if ( NOT APPLE )
    add_subdirectory( ap240 )
    add_subdirectory( ap240controller ) 
    add_subdirectory( u5303a )
  endif()
  add_subdirectory( acqrswidgets )  # Qt GUI
endif()

add_subdirectory( acqrsinterpreter )
add_subdirectory( acqrscontrols )
