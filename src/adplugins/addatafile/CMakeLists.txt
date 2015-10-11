cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( addatafile )

set( PROVIDER MS-Cheminformatics )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

add_definitions(-DADDATAFILE_LIBRARY)

add_library( addatafile MODULE 
  addatafile.cpp
  datafile.cpp
  datafile_factory.cpp
  rawdata.cpp
  addatafile.hpp
  addatafile_global.h
  datafile.hpp
  datafile_factory.hpp
  rawdata.hpp
  )

target_link_libraries( addatafile LINK_PUBLIC
  ${Boost_LIBRARIES}
  adplugin
  acewrapper
  adcontrols
  adlog
  adfs
  adportable
  adutils
  portfolio
  xmlparser
  adplugin
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