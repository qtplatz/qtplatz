cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_json )

find_package( Qt5 OPTIONAL_COMPONENTS Core QUIET )

add_definitions( -DDATAFILE="${CMAKE_SOURCE_DIR}/data.json" )

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}  
  )

add_executable( ${PROJECT_NAME}
  boost_json.cpp
  boost_json.hpp
  qt5_json.cpp
  qt5_json.hpp
  nlohmann_json.cpp
  nlohmann_json.hpp
  rapidjson_json.cpp
  rapidjson_json.hpp
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  Qt5::Core
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 )

#target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} )


