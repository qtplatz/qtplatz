cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( batchproc )
set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Widgets PrintSupport Svg Sql Script Network ) 
find_package( Boost REQUIRED COMPONENTS system date_time regex thread filesystem iostreams )

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

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec  
  )

add_definitions(
  -DBATCHPROC_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

message( "Qt5_INCLUDE_DIRS: "   ${Qt5Widgets_INCLUDE_DIRS} )

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}  
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( SOURCES
  batchprocplugin.cpp
  mainwindow.cpp 
  batchmode.cpp 
  droptargetform.cpp
  dropwidget.cpp
  batchprocdelegate.cpp
  task.cpp
  import.cpp
  process.cpp
  datainterpreter.cpp
  massspectrometerfactory.cpp
  massspectrometer.cpp
  )
  
set( HEADERS
  batchprocplugin.hpp
  batchproc_global.hpp
  batchprocconstants.hpp
  mainwindow.hpp
  batchmode.hpp
  droptargetform.hpp
  dropwidget.hpp
  batchprocdelegate.hpp
  task.hpp
  import.hpp
  importdata.hpp
  process.hpp
  datainterpreter.hpp
  massspectrometerfactory.hpp
  massspectrometer.hpp
  )

set( RESOURCES batchproc.qrc )
set( FORMS droptargetform.ui )

add_library( batchproc MODULE
  ${SOURCES}
  ${HEADERS}
  ${RESOURCES}
  )

if ( APPLE )
  set( platform_libs bz2 )
elseif( ${CMAKE_SYSTEM} MATCHES "Linux" )
  set( platform_libs bz2 dl )
endif()

target_link_libraries(
  batchproc LINK_PUBLIC
  adplugin
  adcontrols
  adlog
  adutils
  portfolio
  adfs
  adextension
  adportable
  Core
  extensionsystem
  utils
  ${QWT_LIBRARIES}
  ${Boost_LIBRARIES}
  ${platform_libs}
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


