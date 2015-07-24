
if ( WIN32 )
  find_path( COMPASSDIR NAMES CompassXtract
    PATHS "C:/Program Files/Bruker Daltonik/CompassXtract" )
  if ( COMPASSDIR )
    add_subdirectory( compassxtract )
  endif()
endif()

add_subdirectory( fticr )
