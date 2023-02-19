
add_subdirectory (adurlx)
add_subdirectory (eventbroker)

if ( NOT CMAKE_CROSSCOMPILING )

  if ( AqDrv4_FOUND )
    add_subdirectory (adfile)
  endif()
  if ( NOT WIN32 AND AqDrv4_FOUND )
    add_subdirectory (adimport) #-- issue (direct liniing with adtextfiles .cpp files) need to be resolved
  endif()
  if ( AqDrv4_FOUND )
    add_subdirectory (adexport)
  endif()
  #  add_subdirectory (adhist)

  if ( QT_FOUND )

    add_subdirectory (eventtool)
    add_subdirectory (sdfview)

    if ( HDF5_FOUND )
      add_subdirectory (counting)
    endif()

    if ( OpenCV_FOUND AND HDF5_FOUND )
      add_subdirectory (counting2d)
    endif()

    if ( NOT WIN32 AND AqDrv4_FOUND )
      add_subdirectory (acqiris)
    endif()

  endif()
endif()
