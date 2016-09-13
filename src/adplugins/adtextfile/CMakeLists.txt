cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( adtextfile )
set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

find_package( Qt5 CONFIG REQUIRED Core Widgets )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

#include_directories( ${Qt5Core_INCLUDE_DIRS}  )
include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_definitions(-DADTXTFACTORY_LIBRARY)

add_library( adtextfile MODULE 
  adtextfile.cpp
  adtextfile.hpp
  datafile.cpp
  datafile.hpp
  datafile_factory.cpp
  datafile_factory.hpp
  time_data_reader.cpp
  time_data_reader.hpp
  txtspectrum.cpp
  txtspectrum.hpp
  txtchromatogram.cpp
  txtchromatogram.hpp
  dialog.cpp
  dialog.hpp
  )

target_link_libraries( adtextfile LINK_PUBLIC
  ${Boost_LIBRARIES}
  adcontrols
  adlog
  adportable
  xmlparser
  portfolio
  adplugin
  adfs
  Qt5::Core
  Qt5::Widgets
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
