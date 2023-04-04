cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( addatafile )

set( PROVIDER MS-Cheminformatics )

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
  )

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
  LIBRARY_OUTPUT_DIRECTORY ${_adplugin_output_dir} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  LIBRARY DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

install( FILES ${PROJECT_NAME}.adplugin
  DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins )
