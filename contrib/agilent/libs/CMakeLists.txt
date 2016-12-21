
if ( AqDrv4_FOUND )
  # if AqDrv4 driver installed
  add_subdirectory( ap240 )
  add_subdirectory( ap240controller )
endif()

if ( AgMD2_FOUND )
  add_subdirectory( u5303a )
endif()

if ( WITH_QT5 )
  add_subdirectory( acqrswidgets )  # Qt GUI
endif()

add_subdirectory( acqrsinterpreter )
add_subdirectory( acqrscontrols )
add_subdirectory( aqdrv4controller )  # Remote access to AP240/DC122

