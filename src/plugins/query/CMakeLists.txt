cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( Query )
set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Network )

find_package( qwt )

include_directories( ${QWT_INCLUDE_DIRS} )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format
  -output ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

add_definitions(
  -DQUERY_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

add_library( Query MODULE
  document.cpp
  document.hpp
  mainwindow.cpp
  mainwindow.hpp
  countingquerydialog.cpp
  countingquerydialog.hpp
  plotdialog.cpp
  plotdialog.hpp
  queryconnection.cpp
  queryconnection.hpp
  queryeditor.cpp
  queryeditor.hpp
  queryfactory.cpp
  queryfactory.hpp
  querymode.cpp
  querymode.hpp
  queryplugin.cpp
  queryplugin.hpp
  queryform.cpp
  queryform.hpp
  querywidget.cpp
  querywidget.hpp
  queryresulttable.cpp
  queryresulttable.hpp
  sqledit.cpp
  sqledit.hpp
  sqlhistorydialog.cpp
  sqlhistorydialog.hpp
  ${Resources}
  ${_charts_sources}
  )

target_link_libraries(
  Query LINK_PUBLIC
  adplugin
  adcontrols
  adacquire
  adlog
  adutils
  portfolio
  adfs
  adprot
  adextension
  adplot
  adwidgets
  adportable
  chromatogr
  qtwrapper
  xmlparser
  Core
  extensionsystem
  ${Boost_LIBRARIES}
  ${_opencv_libraries}
  ${_charts_libraries}
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
  DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
