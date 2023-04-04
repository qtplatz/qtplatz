cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )

project (infitofprocessor)

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

add_definitions( -DINFITOFPROCESSOR_LIBRARY )
find_package( Threads )

if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( infitofprocessor MODULE
  addcontextmenu.cpp
  addcontextmenu.hpp
  calibscanlaw.cpp
  calibscanlaw.hpp
  dataprocessor.cpp
  dataprocessor.hpp
  estimatescanlaw.cpp
  estimatescanlaw.hpp
  infitofprocessor.cpp
  nlapdeconv.hpp
  nlapdeconv.cpp
  oncreate.cpp
  oncreate.hpp
  processreactor.cpp
  processreactor.hpp
)

target_link_libraries( infitofprocessor LINK_PUBLIC
  acewrapper
  adcontrols
  adfs
  adplugin
  adportable
  adprocessor
  portfolio
  admtcontrols
  infitofwidgets
  ${Boost_LIBRARIES}
  Threads::Threads
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  LIBRARY DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

install( FILES ${PROJECT_NAME}.adplugin
  DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins )
