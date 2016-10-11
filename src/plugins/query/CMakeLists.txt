cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( Query )
set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Network )

find_package( qwt )
if ( NOT Boost_FOUND )
  find_package( Boost REQUIRED COMPONENTS system date_time regex thread filesystem wserialization serialization )
endif()

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
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec  
  )

add_definitions(
  -DQUERY_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( query_SOURCES
  mainwindow.cpp
  mainwindow.hpp
  queryconnection.cpp
  queryconnection.hpp
  querydocument.cpp
  querydocument.hpp
  queryeditor.cpp
  queryeditor.hpp
  queryfactory.cpp
  queryfactory.hpp
  querymode.cpp
  querymode.hpp
  queryplugin.cpp
  queryplugin.hpp
  queryquery.cpp
  queryquery.hpp  
  queryqueryform.cpp
  queryqueryform.hpp  
  queryquerywidget.cpp
  queryquerywidget.hpp
  queryresulttable.cpp
  queryresulttable.hpp
  sqledit.cpp
  sqledit.hpp
  )

set( query_FORMS doubletabwidget.ui  queryconfigform.ui )

add_library( Query MODULE
  ${query_SOURCES}
  ${Resources}
  )

target_link_libraries(
  Query LINK_PUBLIC
  adplugin
  adcontrols
  adicontroller
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
  ${QWT_LIBRARIES}
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
  DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
