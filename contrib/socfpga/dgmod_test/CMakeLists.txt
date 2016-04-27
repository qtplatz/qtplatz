cmake_minimum_required (VERSION 2.8.11)
cmake_policy(SET CMP0028 NEW)

project (dgmod_test)

set( CMAKE_INCLUDE_CURRENT_DIR ON )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_path( _boost NAMES include/boost HINTS "/usr/local" "/usr/local/boost-1_59" )
if ( _boost )
  set(Boost_INCLUDE_DIR "${_boost}/include")
  set(Boost_LIBRARY_DIR "${_boost}/lib")
endif()

find_package( Boost 1.58 REQUIRED COMPONENTS system )

include_directories(  ${CMAKE_SOURCE_DIR}/../drivers  )

add_executable( ${PROJECT_NAME}
  main.cpp
  )

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  ${Boost_LIBRARIES}
)

