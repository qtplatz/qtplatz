cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0022 NEW )
project ( multumwidgets)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 REQUIRED Core Gui PrintSupport Sql Svg Widgets )
find_package( Threads REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/contrib/multum/libs
  ${CMAKE_SOURCE_DIR}/contrib/include
  ${QWT_DIR}/include
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_definitions( -DMULTUMWIDGETS_LIBRARY )

if ( WIN32 )
  add_definitions( -wd4996 )
endif()

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

add_library( ${PROJECT_NAME} SHARED
  multumwidgets_global.hpp
  protocolform.cpp
  protocolform.hpp
  protocolwidget.cpp
  protocolwidget.hpp
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  multumcontrols
  adwidgets
  adcontrols
  adportable
  adlog
  adplugin
  adurl
  adio
  qtwrapper
  Qt5::Sql
  Qt5::Svg
  Qt5::Widgets
  ${Boost_LIBRARIES}
  Threads::Threads
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
