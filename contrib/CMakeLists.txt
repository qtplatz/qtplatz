cmake_minimum_required (VERSION 2.8.9)

add_subdirectory( shrader )

if (WIN32)
  add_subdirectory( agilent )    # depend on COM
  add_subdirectory( installer )
else()
  message( STATUS "You may need to create independent package for 'boost, qt5 and tao'" )
endif()
