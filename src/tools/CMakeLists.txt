
add_subdirectory (eventbroker)
add_subdirectory (eventtool)
add_subdirectory (counting)

if ( OpenCV_FOUND )
  add_subdirectory (counting2d)
endif()

if ( NOT APPLE )
  add_subdirectory (acqiris)
endif()


