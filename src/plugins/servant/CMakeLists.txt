cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( servant )

set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Network Script Sql )

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY   ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/MS-Chrminformatics ) #plugins

set(CMAKE_AUTOMOC ON)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/servant.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/servant.pluginspec
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

add_library( servant SHARED
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
  adlog
  qtwrapper
  adextension
  xmlparser
  Core
  extensionsystem
  Qt5::Core
)

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES ${PROJECT_NAME}.pluginspec DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROVIDER} )

#  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )      
