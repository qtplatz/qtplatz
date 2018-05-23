######
## Eigen3 install on Windows
##

if ( NOT OPENCV_SOURCE_DIR )
  messge( FATAL_ERROR ${OPENCV_SOURCE_DIR} )
endif()
if ( NOT OPENCV_CONTRIB_SOURCE_DIR )
  messge( FATAL_ERROR ${OPENCV_CONTRIB_SOURCE_DIR} )
endif()
if ( NOT OPENCV_EXTRA_SOURCE_DIR )
  messge( FATAL_ERROR ${OPENCV_EXTRA_SOURCE_DIR} )
endif()
if ( NOT OPENCV_BINARY_DIR )
  messge( FATAL_ERROR ${OPENCV_BINARY_DIR} )
endif()

message( STATUS "OPENCV_SOURCE_DIR         = " ${OPENCV_SOURCE_DIR} )
message( STATUS "OPENCV_CONTRIB_SOURCE_DIR = " ${OPENCV_CONTRIB_SOURCE_DIR} )
message( STATUS "OPENCV_EXTRA_SOURCE_DIR   = " ${OPENCV_EXTRA_SOURCE_DIR} )
message( STATUS "OPENCV_BINARY_DIR         = " ${OPENCV_BINARY_DIR} )

if ( NOT EXISTS ${OPENCV_SOURCE_DIR} )
  execute_process( COMMAND git clone https://github.com/opencv/opencv.git ${OPENCV_SOURCE_DIR} )
endif()

if ( NOT EXISTS ${OPENCV_CONTRIB_SOURCE_DIR} )
  execute_process( COMMAND git clone https://github.com/opencv/opencv_contrib.git ${OPENCV_CONTRIB_SOURCE_DIR} )
endif()

if ( NOT EXISTS ${OPENCV_EXTRA_SOURCE_DIR} )
  execute_process( COMMAND git clone https://github.com/opencv/opencv_extra.git ${OPENCV_EXTRA_SOURCE_DIR} )
endif()

if ( NOT EXISTS ${OPENCV_BINARY_DIR} )
  file( MAKE_DIRECTORY ${OPENCV_BINARY_DIR} )
endif()

if ( NOT EXISTS ${OPENCV_BINARY_DIR}/LICENSE.txt )
  configure_file(
    ${OPENCV_SOURCE_DIR}/LICENSE.txt
    ${OPENCV_BINARY_DIR}/LICENSE.txt
    COPYONLY )
endif()

