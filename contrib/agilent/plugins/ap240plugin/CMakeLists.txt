cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( ap240plugin )

set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

#QT += core svg printsupport widgets
#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets Sql Svg Network PrintSupport )
find_package( qwt )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB ap240_Resources *.qrc)

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
  -DAP240_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  -D_LINUX
  -D_ACQIRIS
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtWidgets_INCLUDE_DIRS}
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

# ap240 files

add_library(
  ap240plugin MODULE
  ap240_constants.hpp
  ap240_global.hpp
  ap240plugin.cpp
  ap240plugin.hpp
  ap240mode.cpp
  ap240mode.hpp
  constants.cpp
  constants.hpp
  document.cpp
  document.hpp
  icontrollerimpl.cpp
  icontrollerimpl.hpp
  mainwindow.cpp
  mainwindow.hpp
  resultwriter.cpp
  resultwriter.hpp
  task.cpp
  task.hpp
  tdcdoc.cpp
  tdcdoc.hpp
  waveformwnd.cpp
  waveformwnd.hpp
  ${ap240_Resources}
  )

target_link_libraries(
  ap240plugin LINK_PUBLIC
  adfs
  adurl
  adutils
  adcontrols
  adacquire
  adplot
  adwidgets
  acewrapper
  adplugin
  adextension
  adlog
  adportable
  xmlparser
  Core
  ap240
  acqrscontrols
  acqrswidgets
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
