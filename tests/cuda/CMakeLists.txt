cmake_minimum_required( VERSION 3.5 )
project( cudaQuery )

set( CMAKE_CXX_STANDARD 14 )

if ( CUDA_FOUND )
  list( APPEND CUDA_NVCC_FLAGS "-std=c++11;-O2;-DVERBOSE")
  set ( CUDA_PROPAGATE_HOST_FLAGS OFF )
endif()

include_directories(
  ${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc
  ${CUDA_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

cuda_add_executable( ${PROJECT_NAME}
  main.cpp
  test.cu
  )

target_link_libraries( ${PROJECT_NAME} ${CUDA_LINK_LIBRARIES_KEYWORD}
  ${CUDA_LIBRARIES}
  ${Boost_LIBRARIES}
  cuda
)
