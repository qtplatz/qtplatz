cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( Query )
set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Network )

#if ( "${Qt5_VERSION}" VERSION_LESS "5.6" )
#  find_package( Qt5 CONFIG REQUIRED WebKitWidgets )  
#else()
#  find_package( Qt5 CONFIG REQUIRED WebEngineWidgets )
#endif()

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

set(COMPAT_VERSION 3.2.81)
set(VERSION 3.1.4)

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
  queryconnection.cpp
  querydocument.cpp
  queryeditor.cpp
  queryfactory.cpp
  querymode.cpp
  queryplugin.cpp
  queryquery.cpp  
  queryqueryform.cpp  
  queryquerywidget.cpp
  queryresulttable.cpp
  )

set( query_HEADERS
  mainwindow.hpp
  queryconnection.hpp  
  querydocument.hpp  
  queryeditor.hpp
  queryfactory.hpp
  querymode.hpp
  queryplugin.hpp
  queryquery.hpp  
  queryqueryform.hpp  
  queryquerywidget.hpp
  queryresulttable.hpp  
  )

set( query_RESOURCES query.qrc )
set( query_FORMS doubletabwidget.ui  queryconfigform.ui )

add_library( Query MODULE
  ${query_SOURCES}
  ${query_HEADERS}
  ${query_RESOURCES}
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
