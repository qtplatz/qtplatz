cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( adtextfile )
set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

include_directories( ${Qt5Core_INCLUDE_DIRS}  )

add_definitions(-DADTXTFACTORY_LIBRARY)

add_library( adtextfile MODULE 
  adtextfile.cpp
  datafile.cpp
  datafile_factory.cpp
  txtspectrum.cpp
  txtchromatogram.cpp
  adtextfile.hpp
  datafile.hpp
  datafile_factory.hpp
  txtspectrum.hpp
  txtchromatogram.hpp
  )

target_link_libraries( adtextfile LINK_PUBLIC
  ${Boost_LIBRARIES}
  adcontrols
  adlog
  adportable
  xmlparser
  portfolio
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