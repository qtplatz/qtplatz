cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )
cmake_policy( SET CMP0028 NEW )

project( compassxtract )

set( PROVIDER MS-Cheminformatics )

configure_file(
  "${CMAKE_SOURCE_DIR}/src/adplugin.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  @ONLY
  )

set( COMPASSDIR "C:/Program Files/Bruker Daltonik/CompassXtract" )

include_directories( ${COMPASSDIR} )

add_definitions( -DCOMPASSXTRACT_LIBRARY )
if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( ${PROJECT_NAME} MODULE
  compassxtract.cpp
  datafile_factory.cpp
  datafile.cpp
  safearray.cpp
  compassxtract.hpp
  compassxtract_global.hpp
  datafile_factory.hpp
  datafile.hpp
  safearray.hpp
  )

target_link_libraries( compassxtract
  ${Boost_LIBRARIES}
  adplugin
  adportable
  adlog
  adcontrols
  portfolio
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


