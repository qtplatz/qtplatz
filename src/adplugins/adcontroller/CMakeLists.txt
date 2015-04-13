cmake_policy( SET CMP0028 NEW )

project( adcontroller )

set( PROVIDER MS-Cheminformatics )

add_definitions(-DADCONTROLLER_LIBRARY)

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

find_package( ace+tao )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

include_directories(
  ${CMAKE_BINARY_DIR}/src/libs/adinterface
  ${CMAKE_BINARY_DIR}/src/libs
  ${ACE+TAO_INCLUDE_DIRS}
  )

link_directories( ${ACE+TAO_LIBRARY_DIRS} )
message( "TAO_LIBRARY_DIRS" ${TAO_LIBRARY_DIRS} )

add_library( adcontroller MODULE
  adcontroller.cpp
  cache.cpp
  fileio.cpp
  iproxy.cpp
  manager_i.cpp
  observer_i.cpp
  oproxy.cpp
  receiver_i.cpp
  sampleprocessor.cpp
  session_i.cpp
  task.cpp
  logging.cpp
  adcontroller.hpp
  adcontroller_global.h
  cache.hpp
  fileio.hpp
  iproxy.hpp
  manager_i.hpp
  observer_i.hpp
  oproxy.hpp
  receiver_i.hpp
  sampleprocessor.hpp
  session_i.hpp
  task.hpp
  logging.hpp
  )

if ( CMAKE_SYSTEM_NAME EQUAL Linux )
  set( linux_LIBS dl rt )
endif()

target_link_libraries( adcontroller LINK_PUBLIC
  acewrapper
  adinterface
  adfs
  adcontrols
  adportable
  adutils
  adplugin
  xmlparser
  adlog
  ${ACE+TAO_LIBRARIES}
  ${Boost_LIBRARIES}
  ${linux_LIBS}
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

install( FILES ${PROJECT_NAME}.adplugin DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )