cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( aqmd3plugin )

set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Threads REQUIRED )

find_package( Qt5 CONFIG REQUIRED Core Widgets Sql Svg Network PrintSupport )

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
  ${QTPLATZ_SOURCE_DIR}/contrib/acqiris/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/infitof/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/multum/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/socfpga/libs
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src         # version.h
  )

add_library(
  aqmd3plugin MODULE
  aqmd3_global.hpp
  aqmd3mode.hpp
  aqmd3mode.cpp
  aqmd3plugin.cpp
  aqmd3plugin.hpp
  constants.cpp
  constants.hpp
  document.cpp
  document.hpp
  iaqmd3impl.cpp
  iaqmd3impl.hpp
  idgmodimpl.cpp
  idgmodimpl.hpp
  isequenceimpl.cpp
  mainwindow.cpp
  mainwindow.hpp
  task.cpp
  task.hpp
  tdc.cpp
  tdc.hpp
  waveformwnd.cpp
  waveformwnd.hpp
  isequenceimpl.cpp
  isequenceimpl.hpp
  resultwriter.cpp
  resultwriter.hpp
  ${Resources}
  )

target_link_libraries(
  aqmd3plugin LINK_PUBLIC
  adacquire
  adcontrols
  adextension
  adfs
  adlog
  adplot
  adplugin
  adplugin_manager
  adportable
#  adportable_serializer
  aqmd3
  aqmd3widgets
  adurl
  adutils
  adwidgets
  infitofwidgets
  admtwidgets
  qtwrapper
  socfpga
  xmlparser
  Core
  Qt5::Svg
  Qt5::Widgets
  Threads::Threads
#  ${Boost_LIBRARIES}
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
