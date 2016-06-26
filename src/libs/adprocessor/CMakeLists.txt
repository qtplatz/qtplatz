
project(adprocessor)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 CONFIG REQUIRED Core Svg Sql Widgets PrintSupport )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
  ${Qt5Sql_INCLUDE_DIRS}  
  ${Qt5Widgets_INCLUDE_DIRS}
  )

if ( rdkit_FOUND )
  include_directories( ${RDKit_INCLUDE_DIRS} )
endif()

add_definitions(-DADPROCESSOR_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251" )
endif()

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( _sources "" )
if ( qwt_FOUND )
  set ( _sources pressuregauge.cpp )
endif()

add_library( ${PROJECT_NAME} SHARED
  dataprocessor.hpp
  dataprocessor.cpp
  )

target_link_libraries( adprocessor LINK_PUBLIC
  ${QWT_LIBRARIES}
  Qt5::Core
  Qt5::Widgets
  Qt5::Svg
  Qt5::Sql
  Qt5::PrintSupport
  ${Boost_LIBRARIES}
  ${RDKit_LIBRARIES}
  )

set_property( TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14 )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
