cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )

project (infitofspectrometer)

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
  ${AqDrv4_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/multum/libs
  ${QtPrintSupport_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
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
  admtcontrols
  ${Boost_LIBRARIES}
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
  )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION "lib/qtplatz" COMPONENT libraries
)

install( FILES "${OUTPUT_DIR}/${PROJECT_NAME}.adplugin" DESTINATION "${_adplugin_install_dir}" COMPONENT plugins )
