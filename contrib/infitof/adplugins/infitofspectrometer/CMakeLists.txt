cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0022 NEW )

project (infitofspectrometer)

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

## find_package( acqiris )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

include_directories(
  ${AqDrv4_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/multum/libs
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_definitions( -DINFITOFSPECTROMETER_LIBRARY )

if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( infitofspectrometer
  MODULE
  ap240translator.hpp
  constants.cpp
  constants.hpp
  infitofdatainterpreter.cpp
  infitofdatainterpreter.hpp
  infitofspectrometer.cpp
  infitofspectrometer_global.h
  massspectrometer.cpp
  massspectrometer.hpp
  textfile.cpp
  textfile.hpp
  u5303a_translator.cpp
  u5303a_translator.hpp
  ap240translator.cpp
  ap240translator.hpp
  infitofdefns/avgrdata.cpp
  infitofdefns/avgrdata.hpp
  infitofdefns/avgr_acqiris.cpp
  infitofdefns/avgr_acqiris.hpp
  infitofdefns/avgr_u5303a.cpp
  infitofdefns/avgr_u5303a.hpp
  infitofdefns/serializer.cpp
  infitofdefns/serializer.hpp
)

target_link_libraries( infitofspectrometer LINK_PUBLIC
  infitofcontrols
  adplugin
  adportable
  adcontrols
  acewrapper
  acqrscontrols
  adfs
  multumcontrols
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
