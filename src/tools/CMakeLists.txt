
add_subdirectory (eventbroker)
add_subdirectory (eventtool)
add_subdirectory (counting)

if ( CUDA_FOUND )
#  add_subdirectory (cuda)
endif()

if ( OpenCV_FOUND )
  add_subdirectory (counting2d)
endif()

if ( NOT WIN32 )
  add_subdirectory (acqiris)
endif()


