cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( addatafile )

set( PROVIDER MS-Cheminformatics )

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
  "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
)

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )
# should be './bin/qtplatz.app/Contents/PlugIns/MS-Cheminformatics/libadtextfile.dylib'

message( STATUS "############ OUTPUT_DIR:               ${_adplugin_output_dir}" )
message( STATUS "############ OUTPUT_DIR:               ${OUTPUT_DIR}" )

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
  LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
#  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
#  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_adplugin_output_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_adplugin_output_dir}" COMPONENT plugins
  ARCHIVE DESTINATION "lib/qtplatz" COMPONENT libraries
#  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
#  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
)

install( FILES ${PROJECT_NAME}.adplugin DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
