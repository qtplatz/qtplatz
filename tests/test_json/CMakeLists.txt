
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( test_json )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#find_package( Qt5 CONFIG REQUIRED Core )
#find_package( Qt5 OPTIONAL_COMPONENTS Core QUIET )
find_package( Qt5 CONFIG REQUIRED Core Widgets )
if ( Qt5_FOUND )
  add_definitions( -DHAVE_Qt5_JSON=1 )
  set ( Qt5_JSON_SOURCES   qt5_json.cpp  qt5_json.hpp )
endif()

find_file ( RAPIDJSON_DIR rapidjson "/usr/local/include" )
find_file ( NLOHMANN_DIR nlohmann "/usr/local/include" )

if ( RAPIDJSON_DIR )
  add_definitions( -DHAVE_RAPIDJSON_JSON=1 )
  set ( RAPIDJSON_JSON 1 )
  set ( RAPIDJSON_JSON_SOURCES  rapidjson_json.cpp rapidjson_json.hpp )
  message( STATUS "Found RAPIDJSON in ", ${RAPIDJSON_DIR} )
endif()

if ( NLOHMANN_DIR )
  add_definitions( -DHAVE_NLOHMANN_JSON=1 )
  set ( NLOHMANN_JSON 1 )
  set ( NLOHMANN_JSON_SOURCES nlohmann_json.cpp nlohmann_json.hpp )
  message( STATUS "FOUND NLOHMANN JSON in ", ${NLOHMANN_DIR} )
endif()

add_definitions( -DDATAFILE="${CMAKE_CURRENT_SOURCE_DIR}/data.json" )

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  "/usr/local/include"
  )

add_executable( ${PROJECT_NAME}
  boost_ptree.cpp
  boost_ptree.hpp
  boost_json.cpp
  boost_json.hpp
  ${Qt5_JSON_SOURCES}
  ${NLOHMANN_JSON_SOURCES}
  ${RAPIDJSON_JSON_SOURCES}
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  Qt5::Core
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 )

#target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} )
