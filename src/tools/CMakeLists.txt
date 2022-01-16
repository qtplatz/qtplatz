
add_subdirectory (adurlx)
add_subdirectory (eventbroker)

if ( NOT CMAKE_CROSSCOMPILING )

  add_subdirectory (adfile)
  add_subdirectory (adimport)
  add_subdirectory (adexport)
  #  add_subdirectory (adhist)

  if ( Qt5_FOUND )

    add_subdirectory (eventtool)
    add_subdirectory (sdfview)

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
endif()
