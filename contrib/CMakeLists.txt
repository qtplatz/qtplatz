cmake_minimum_required (VERSION 2.8.9)

message( STATUS "------------ contrib/shrader ----------------" )
add_subdirectory( shrader )

message( STATUS "------------ contrib/agilent ----------------" )
add_subdirectory( agilent )

if (WIN32)
  message( STATUS "------------ contrib/bruker ----------------" )  
  add_subdirectory( bruker )
  message( STATUS "------------ contrib/installer (WIN32) ----------------" )  
  add_subdirectory( installer )
elseif( APPLE )
  message( STATUS "------------ contrib/installer (APPLE) ----------------" )    
  add_subdirectory( installer )  
endif()
