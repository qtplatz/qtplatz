project( acquire )
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

find_package( Qt5 CONFIG REQUIRED Core Widgets Sql Script Network )
find_package( qwt )
find_package( ace+tao )
find_package( Boost REQUIRED COMPONENTS system date_time regex filesystem wserialization serialization )

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

link_directories(
  ${ACE+TAO_LIBRARY_DIRS}
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

target_link_libraries(
  acquire LINK_PUBLIC
  adfs
  adutils
  adcontrols
  adinterface
  adplot
  adwidgets
  acewrapper
  adplugin
  adextension
  adorbmgr
  adlog
  adportable
  xmlparser
  Core
  optimized TAO_Utils debug TAO_Utilsd
  optimized TAO_PortableServer debug TAO_PortableServerd
  optimized TAO_AnyTypeCode debug TAO_AnyTypeCoded
  optimized TAO debug TAOd
  optimized ACE debug ACEd
  ${Boost_LIBRARIES}
  )

