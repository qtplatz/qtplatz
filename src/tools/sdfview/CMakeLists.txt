cmake_minimum_required( VERSION 3.5 )
project( sdfview )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

set ( STANDALONE OFF CACHE BOOL "STANDALONE" )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set ( USING_CHARTS OFF )

if ( STANDALONE )
  include( "standalone.cmake" )
endif()

include( version )
set ( VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )

find_package( Qt5 CONFIG REQUIRED Widgets Gui Svg Sql PrintSupport )
find_package( qwt )

add_definitions( -DPID_NAME="/var/run/${PROJECT_NAME}.pid" )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${RDKit_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
  ${Qt5Sql_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/app           # app_version.h
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  document.cpp
  document.hpp
  outputwidget.cpp
  outputwidget.hpp
  moltablewnd.cpp
  moltablewnd.hpp
  moltabledelegate.cpp
  moltabledelegate.hpp
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adchem
  adcontrols
  adfs
  adportable
  adui
  adwidgets
  utils
  Core
  Qt5::Widgets
  Qt5::Svg
  Qt5::Sql
  ${Boost_LIBRARIES}
  ${RDKit_LIBRARIES}
  ${CMAKE_DL_LIBS}
  )

install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT applications )
