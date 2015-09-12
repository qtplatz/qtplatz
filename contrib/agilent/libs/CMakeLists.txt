
if ( WIN32 )
  add_subdirectory( u5303a ) # Using COM
endif()

if ( NOT CMAKE_CROSSCOMPILING )
  if ( NOT APPLE )
    add_subdirectory( ap240 )
    add_subdirectory( ap240controller )  
  endif()
  add_subdirectory( ap240w )
endif()

add_subdirectory( u5303aspectrometer )
add_subdirectory( ap240controls )
