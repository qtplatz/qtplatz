
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( boost_json )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include ("${CMAKE_CURRENT_SOURCE_DIR}/../../boost.cmake")
find_package( Boost 1.75 REQUIRED COMPONENTS json )

include_directories(
  ${Boost_INCLUDE_DIRS}
  "/usr/local/include"
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 )
