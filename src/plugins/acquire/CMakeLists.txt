cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( acquireplugin )

set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Threads REQUIRED )

#QT += core svg printsupport widgets
find_package( Qt5 REQUIRED Core Gui PrintSupport Sql Svg Widgets )

find_package( qwt )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

include_directories(
  ${QTPLATZ_SOURCE_DIR}/contrib/socfpga/libs
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src         # version.h
  )

add_library( ${PROJECT_NAME} MODULE
  acquire_global.hpp
  mode.hpp
  mode.cpp
  acquireplugin.cpp
  acquireplugin.hpp
  constants.cpp
  constants.hpp
  document.cpp
  document.hpp
  isequenceimpl.cpp
  mainwindow.cpp
  mainwindow.hpp
  task.cpp
  task.hpp
  waveformwnd.cpp
  waveformwnd.hpp
  idgmodimpl.cpp
  idgmodimpl.hpp
#  isequenceimpl.cpp
#  isequenceimpl.hpp
#  resultwriter.cpp
#  resultwriter.hpp
  ${Resources}
  )

target_link_libraries(
  acquireplugin LINK_PUBLIC
  adacquire
  adcontrols
  adextension
  adfs
  adlog
  adplot
  adplugin
  adplugin_manager
  adportable
  adurl
  adutils
  adwidgets
  qtwrapper
  Core
  extensionsystem
  socfpga
  utils
  Qt5::Svg
  Qt5::Widgets
  Threads::Threads
  xmlparser
  ${Boost_LIBRARIES}
  ${QWT_LIBRARIES}
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
  DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  )
