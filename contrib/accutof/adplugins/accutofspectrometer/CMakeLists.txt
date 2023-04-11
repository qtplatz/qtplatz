cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )

project (accutofspectrometer)

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )
qtplatz_adplugin_install_dir( _adplugin_install_dir ${PROVIDER} )

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
  "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
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
  LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION "lib/qtplatz" COMPONENT libraries
)

install( FILES "${OUTPUT_DIR}/${PROJECT_NAME}.adplugin" DESTINATION "${_adplugin_install_dir}" COMPONENT plugins )
