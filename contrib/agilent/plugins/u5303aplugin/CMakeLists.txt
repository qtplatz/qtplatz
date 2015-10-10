cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( u5303aplugin )

set( PROVIDER MS-Cheminformatics )

#QT += core svg printsupport widgets
find_package( Qt5 CONFIG REQUIRED Core Widgets Sql Svg Script Network )
find_package( qwt )
find_package( Boost REQUIRED COMPONENTS system date_time regex filesystem wserialization serialization )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB u5303a_Resources *.qrc)

set(COMPAT_VERSION 3.2.81)
set(VERSION 3.1.4)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

add_definitions(
  -DU5303A_LIBRARY
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

# u5303 files

add_library(
  u5303aplugin MODULE
  constants.cpp
  constants.hpp
  document.cpp
  document.hpp
  icontrollerimpl.cpp
  icontrollerimpl.hpp
  isequenceimpl.cpp
  mainwindow.cpp
  mainwindow.hpp
  task.cpp
  task.hpp
  tdcdoc.cpp
  tdcdoc.hpp
  u5303a_global.hpp
  u5303a_constants.hpp
  u5303amode.hpp
  u5303amode.cpp
  u5303aplugin.cpp
  u5303aplugin.hpp
  waveformwnd.cpp
  waveformwnd.hpp
  isequenceimpl.cpp
  isequenceimpl.hpp
  ${u5303a_Resources}
  )

target_link_libraries(
  u5303aplugin LINK_PUBLIC
  adfs
  adutils
  adcontrols
  adplot
  adwidgets
  acewrapper
  adplugin
  adextension
  adlog
  adportable
  acqrswidgets
  xmlparser
  Core
  u5303a
  acqrscontrols
  u5303aspectrometer
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )
  
