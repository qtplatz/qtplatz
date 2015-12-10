cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( acquire )

set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Widgets Sql Network PrintSupport Svg )
find_package( qwt )
if ( QTPLATZ_SUPPORT_CORBA )
  find_package( ace+tao )
endif()
find_package( Boost REQUIRED COMPONENTS system date_time regex filesystem wserialization serialization )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.config"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.config"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec
  )

add_definitions( -DACQUIRE_LIBRARY )

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

include_directories(
  ${CMAKE_BINARY_DIR}/src/libs/adinterface
  ${CMAKE_BINARY_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${ACE+TAO_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

if (WIN32)
  set( LINK_DIRS ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES Darwin )
  set( LINK_DIRS ${QTPLATZ_SOURCE_DIR}/bin/qtplatz.app/Contents/Plugins/QtProject )
else()
  set( LINK_DIRS ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject )
endif()

link_directories(
  ${LINK_DIRS}
  )

set( ${PROJECT_NAME}_SOURCES
  acquiremode.cpp
  acquiremode.hpp
  acquireplugin.cpp
  acquireplugin.hpp
  constants.cpp
  constants.hpp
  document.cpp
  document.hpp
  mainwindow.cpp
  mainwindow.hpp  
  task.cpp
  task.hpp
  session.cpp
  session.hpp
  mastercontroller.cpp
  mastercontroller.hpp
  masterobserver.cpp
  masterobserver.hpp
  masterreceiver.cpp
  masterreceiver.hpp
  masterobserverevents.cpp
  masterobserverevents.hpp  
  waveformwnd.cpp
  waveformwnd.hpp
  )


set( ${PROJECT_NAME}_HEADERS
  acquire_global.h
  )

if ( QTPLATZ_SUPPORT_CORBA )
  list( APPEND ${PROJECT_NAME}_SOURCES
    qbroker.cpp
    qbroker.hpp
    orbconnection.cpp
    orbconnection.hpp
    orb_i.cpp
    orb_i.hpp
    )
  
  set( CORBA_DEPENDENCY
    adinterface
    adorbmgr
    ${ACE+TAO_LIBRARIES}
    )
  
endif()

add_library(
  acquire MODULE
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
  ${Resources}
  )


target_link_libraries(
  acquire LINK_PUBLIC
  adfs
  adutils
  adcontrols
  adicontroller
  adplot
  adwidgets
  acewrapper
  adplugin
  adextension
  adlog
  adportable
  xmlparser
  Core
  ${CORBA_DEPENDENCY}
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
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.config"
  DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )

#"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
