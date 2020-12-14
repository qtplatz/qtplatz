cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )

project (accutofspectrometer)

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/accutof/libs
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  )

add_definitions( -DACCUTOFSPECTROMETER_LIBRARY )

if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( ${PROJECT_NAME} MODULE
  accutofspectrometer.cpp
  datainterpreter.cpp
  datainterpreter.hpp
  massspectrometer.cpp
  massspectrometer.hpp
  scanlaw.cpp
  scanlaw.hpp
  )

#foreach( lib ${Boost_LIBRARIES} )
#  message( STATUS "adspectrometer boost lib: " ${lib} )
#endforeach()

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  accutofcontrols
  acqrscontrols
  adcontrols
  adplugin
  adportable
  adfs
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
