cmake_minimum_required(VERSION 3.30)

project( test_pge2 )
set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )

find_package( Eigen3 REQUIRED )
set( CMAKE_CXX_STANDARD 20 )

include_directories(
  ${EIGEN3_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

message( STATUS "# Eigen3                : ${EIGEN3_INCLUDE_DIR}" )
message( STATUS "# lib dir               : ${CMAKE_CURRENT_SOURCE_DIR}/../../src" )

add_executable( test_pge2
  pge2.cpp
)

add_executable( test_retention
  retention.cpp
)
