
if ( WIN32 )
  add_subdirectory( u5303a ) # Using COM
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT APPLE )
  add_subdirectory( ap240 )
  add_subdirectory( ap240w )
  add_subdirectory( ap240controller )  
endif()

add_subdirectory( u5303aspectrometer )
add_subdirectory( ap240spectrometer )
