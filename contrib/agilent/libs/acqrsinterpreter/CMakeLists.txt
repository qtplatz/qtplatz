cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( acqrsinterpreter )

set( PROVIDER MS-Cheminformatics )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

if ( MSVC )
  add_definitions(-DACQRSINTERPRETER_LIBRARY -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( ${PROJECT_NAME}
  MODULE
  acqrsinterpreter_global.hpp
  datareader.cpp
  datareader.hpp
  datareader_factory.cpp
  datareader_factory.hpp
  acqrsinterpreter.cpp
  massspectrometer.cpp 
  massspectrometer.hpp
  datainterpreter.cpp
  datainterpreter.hpp
  datainterpreter_histogram.cpp
  datainterpreter_histogram.hpp
  datainterpreter_timecount.cpp
  datainterpreter_timecount.hpp
  datainterpreter_softavgr.cpp
  datainterpreter_softavgr.hpp
  datainterpreter_u5303a.cpp
  datainterpreter_u5303a.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin
  adportable
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
