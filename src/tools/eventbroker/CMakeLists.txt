cmake_minimum_required (VERSION 2.8.11)
project (eventbroker)
cmake_policy(SET CMP0022 NEW)

include_directories(${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )
link_directories(${Boost_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib)

add_definitions(-DEVENTBROKER_LIBRARY)

add_library( eventbroker SHARED
  api.cpp 
  singleton.cpp 
  )

