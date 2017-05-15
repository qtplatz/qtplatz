cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0022 NEW )

project (adtofprocessor)

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

include_directories(
  ${ACQRS_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${QTPLATZ_SOURCE_DIR}/contrib/multum/libs
  )

add_definitions( -DADTOFPROCESSOR_LIBRARY )

if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( adtofprocessor MODULE
  addcontextmenu.cpp
  addcontextmenu.hpp
  calibscanlaw.cpp
  calibscanlaw.hpp
  dataprocessor.cpp
  dataprocessor.hpp
  estimatescanlaw.cpp
  estimatescanlaw.hpp
  adtofprocessor.cpp
  oncreate.cpp
  oncreate.hpp
  processreactor.cpp
  processreactor.hpp
)

set_property( TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14 )

target_link_libraries( adtofprocessor LINK_PUBLIC
  adplugin
  adportable
  adcontrols
  acewrapper
  adfs
  adprocessor
  adwidgets
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
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"  
  DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
