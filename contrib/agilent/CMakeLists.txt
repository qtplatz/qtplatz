
add_subdirectory( libs )
add_subdirectory( plugins )
if ( NOT APPLE )
  add_subdirectory( test_u5303a )
endif()



