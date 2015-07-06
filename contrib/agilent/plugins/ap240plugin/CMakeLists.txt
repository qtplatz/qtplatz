cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( ap240plugin )

set( PROVIDER MS-Cheminformatics )

#QT += core svg printsupport widgets
find_package( Qt5 CONFIG REQUIRED Core Widgets Sql Svg Script Network PrintSupport )
find_package( qwt )
find_package( ace+tao )
find_package( Boost REQUIRED COMPONENTS system date_time regex filesystem wserialization serialization )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB ap240_Resources *.qrc)

set(COMPAT_VERSION 3.2.81)
set(VERSION 3.1.4)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

add_definitions(
  -DAP240_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_BINARY_DIR}/src/libs/adinterface
  ${CMAKE_SOURCE_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${ACE+TAO_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

# ap240 files

add_library(
  ap240plugin MODULE
  ap240plugin.cpp
  mainwindow.cpp
  ap240mode.cpp
  document.cpp
  waveformwnd.cpp
  ap240form.cpp
  ap240methodtable.cpp
  ap240methodwidget.cpp
  isequenceimpl.cpp
  ap240plugin.hpp
  ap240_global.hpp
  ap240_constants.hpp
  mainwindow.hpp
  ap240mode.hpp
  document.hpp
  waveformwnd.hpp
  ap240form.hpp
  ap240methodtable.hpp
  ap240methodwidget.hpp
  isequenceimpl.cpp
  ${ap240_Resources}
  )

target_link_libraries(
  ap240plugin LINK_PUBLIC
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
  ap240
  ap240spectrometer
  ap240w
  ${ACE+TAO_LIBRARIES}
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )
  
