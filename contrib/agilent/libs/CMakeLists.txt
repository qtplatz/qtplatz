
if ( NOT CMAKE_CROSSCOMPILING )
  if ( NOT APPLE )
    add_subdirectory( ap240 )
    add_subdirectory( ap240controller ) 
    if ( WIN32 )
      add_subdirectory( u5303a )   # Microsoft COM dependent
    endif()
  endif()
  add_subdirectory( acqrswidgets )  # Qt GUI
endif()

add_subdirectory( u5303aspectrometer )
#add_subdirectory( ap240controls )  # this will be merged into aqrscontrols
add_subdirectory( acqrscontrols )
