cmake_minimum_required (VERSION 3.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_vision )

set( CMAKE_CXX_STANDARD 14 )

find_package( Boost REQUIRED system filesystem program_options )

find_package( Qt5 REQUIRED Core Widgets )

if ( OpenCV_FOUND )
  find_package( OpenCV 3 REQUIRED core imgproc video highgui features2d )
  add_definitions( "-DOPENCV=1" )
  include_directories( ${OpenCV_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} )
endif()

if ( CUDA_FOUND )
  list( APPEND CUDA_NVCC_FLAGS "-std=c++11;-O2;-DVERBOSE")
  set ( CUDA_PROPAGATE_HOST_FLAGS OFF )
  include_directories (
    ${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc
    ${CUDA_INCLUDE_DIRS}
  )
endif()

if ( ArrayFire_FOUND )
  add_definitions( "-DARRAYFIRE=1" )
  include_directories ( ${ArrayFire_INCLUDE_DIRS} )
endif()

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

cuda_add_executable( test_vision
  main.cpp
  increment.cu
  colormap.cu
  )

message( STATUS "CUDA_LIBRARIES: " ${CUDA_LIBRARIES} )

target_link_libraries( test_vision ${CUDA_LINK_LIBRARIES_KEYWORD}
  advision
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${ArrayFire_LIBRARIES}
  ${ArrayFire_CUDA_LIBRARIES}  
  ${CUDA_LIBRARIES}
  Qt5::Core
  Qt5::Widgets  
  )
