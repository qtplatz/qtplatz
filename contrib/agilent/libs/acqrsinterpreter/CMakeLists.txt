cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( acqrsinterpreter )

set( PROVIDER MS-Cheminformatics )

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )
qtplatz_adplugin_install_dir( _adplugin_install_dir ${PROVIDER} )

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

#configure_file(
#  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
#  "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
#)

if ( MSVC )
  add_definitions(-DACQRSINTERPRETER_LIBRARY -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

include_directories( ${PROJECT_SOURCE_DIR}/.. )

add_library( ${PROJECT_NAME}
  MODULE
  acqrsinterpreter_global.hpp
  datareader.cpp
  datareader.hpp
  datareader_factory.cpp
  datareader_factory.hpp
  acqrsinterpreter.cpp
  datainterpreter.cpp
  datainterpreter.hpp
  datainterpreter_histogram.cpp
  datainterpreter_histogram.hpp
  datainterpreter_timecount.cpp
  datainterpreter_timecount.hpp
  datainterpreter_softavgr.cpp
  datainterpreter_softavgr.hpp
  datainterpreter_waveform.cpp
  datainterpreter_waveform.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  acqrscontrols
  adcontrols
  adfs
  adplugin
  adportable
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
