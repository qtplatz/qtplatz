project( acquire )
cmake_policy( SET CMP0020 NEW )

find_package( Qt5 CONFIG REQUIRED Core Widgets )
find_package( qwt )
find_package( ace+tao )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pluginspec"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.config"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.config"
  )

add_definitions(
  -DACQUIRE_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${ACE+TAO_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

set( ${PROJECT_NAME}_SOURCES
  acquiremode.cpp
  acquireplugin.cpp
  orbconnection.cpp
  mainwindow.cpp
  qbroker.cpp
  acquiredocument.cpp
  )

set( ${PROJECT_NAME}_HEADERS
  acquire_global.h
  acquiremode.hpp
  acquireplugin.hpp
  orbconnection.hpp
  mainwindow.hpp
  constants.hpp
  qbroker.hpp
  acquiredocument.hpp
  )

add_library(
  acquire MODULE
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
  ${Resources}
  )



