cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0022 NEW )

project (adtofprocessor)

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
  "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
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

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_adplugin_output_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_adplugin_output_dir}" COMPONENT plugins
  ARCHIVE DESTINATION "lib/qtplatz" COMPONENT libraries
)
