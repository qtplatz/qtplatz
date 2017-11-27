
add_subdirectory (eventbroker)

if ( Qt5_FOUND )

  add_subdirectory (eventtool)

  if ( HDF5_FOUND )
    add_subdirectory (counting)
  endif()

  if ( OpenCV_FOUND AND HDF5_FOUND )
    add_subdirectory (counting2d)
  endif()
  
  if ( NOT WIN32 )
    add_subdirectory (acqiris)
  endif()
  
endif()


