cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )

project (adspectrometer)

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )
qtplatz_adplugin_install_dir( _adplugin_install_dir ${PROVIDER} )

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
  "${OUTPUT_DIR}/${PROJECT_NAME}.adplugin"
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QtPrintSupport_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  )

add_definitions( -DADSPECTROMETER_LIBRARY )

if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( ${PROJECT_NAME} MODULE
  adspectrometer.cpp
  constants.hpp
  importdata.hpp
  massspectrometer.cpp
  massspectrometer.hpp
  )

#foreach( lib ${Boost_LIBRARIES} )
#  message( STATUS "adspectrometer boost lib: " ${lib} )
#endforeach()

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
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
