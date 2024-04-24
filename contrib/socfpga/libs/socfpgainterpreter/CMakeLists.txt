cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( socfpgainterpreter )

set( PROVIDER MS-Cheminformatics )

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )
qtplatz_adplugin_install_dir( _adplugin_install_dir ${PROVIDER} )

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

#configure_file(
#  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
#  "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
#)

if ( MSVC )
  add_definitions(-DADS54JINTERPRETER_LIBRARY -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

include_directories( ${PROJECT_SOURCE_DIR}/.. )

add_library( ${PROJECT_NAME}
  MODULE
  socfpgainterpreter_global.hpp
  datareader.cpp
  datareader.hpp
  datareader_factory.cpp
  datareader_factory.hpp
  socfpgainterpreter.cpp
  datainterpreter.cpp
  datainterpreter.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  socfpga
  adcontrols
  adfs
  adplugin
  adportable
  admethods
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
