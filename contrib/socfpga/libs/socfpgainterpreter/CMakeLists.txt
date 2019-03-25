cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( socfpgainterpreter )

set( PROVIDER MS-Cheminformatics )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

message( STATUS "##### configure adplugin " "${QTPLATZ_PLUGIN_DIRECTORY}" )
message( STATUS "##### configure adplugin " "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}" )

if ( MSVC )
  add_definitions(-DADS54JINTERPRETER_LIBRARY -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

include_directories( ${PROJECT_SOURCE_DIR}/.. )

add_library( ${PROJECT_NAME}
  MODULE
  socfpgainterpreter_global.hpp
  datareader.cpp
  datareader.hpp
  datareader_factory.cpp
  datareader_factory.hpp
  socfpgainterpreter.cpp
  datainterpreter.cpp
  datainterpreter.hpp
#  datainterpreter_waveform.cpp
#  datainterpreter_waveform.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  socfpga
  adcontrols
  adfs
  adplugin
  adportable
  admethods
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

install( FILES ${PROJECT_NAME}.adplugin DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
