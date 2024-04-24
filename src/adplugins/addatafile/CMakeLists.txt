cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( addatafile )

set( PROVIDER MS-Cheminformatics )

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )
qtplatz_adplugin_install_dir( _adplugin_install_dir ${PROVIDER} )

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

#configure_file(
#  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
#  "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
#)

add_definitions(-DADDATAFILE_LIBRARY)

add_library( addatafile MODULE
  addatafile.cpp
  datafile.cpp
  datafile_factory.cpp
  rawdata_v2.cpp
  rawdata_v2.hpp
  rawdata_v3.cpp
  rawdata_v3.hpp
  addatafile.hpp
  addatafile_global.h
  datafile.hpp
  datafile_factory.hpp
  )

target_link_libraries( addatafile LINK_PUBLIC
  ${Boost_LIBRARIES}
  adplugin
  adcontrols
  adlog
  adfs
  adportable
  adutils
  portfolio
  xmlparser
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
