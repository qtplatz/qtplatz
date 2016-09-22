
add_subdirectory (eventbroker)
add_subdirectory (eventtool)
add_subdirectory (counting)

if ( NOT APPLE )
  add_subdirectory (acqiris)
endif()


