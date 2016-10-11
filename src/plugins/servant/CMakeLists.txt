cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( servant )

set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Widgets Network Sql PrintSupport )

set(CMAKE_AUTOMOC ON)

set(COMPAT_VERSION 3.2.81)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"  
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  )

if (WIN32)
  set( servant_LINK_DIRS ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES Darwin )
  set( servant_LINK_DIRS ${QTPLATZ_SOURCE_DIR}/bin/qtplatz.app/Contents/Plugins/QtProject )
else()
  set( servant_LINK_DIRS ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject )
endif()

link_directories(
  ${servant_LINK_DIRS}
  )

add_definitions( -DSERVANT_LIBRARY )

add_library( servant MODULE
  outputwindow.cpp
  servant.cpp
  servantmode.cpp
  servantplugin.cpp
  logger.cpp
  servant_global.h
  outputwindow.hpp
  servant.hpp
  servantmode.hpp
  servantplugin.hpp
  logger.hpp
  )

target_link_libraries( servant LINK_PUBLIC
  ${Boost_LIBRARIES}
  adcontrols
  adextension
  adportable
  adplugin
  adplugin_manager
  adlog
  qtwrapper
  adextension
  xmlparser
  Core
  extensionsystem
  Qt5::Core
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

