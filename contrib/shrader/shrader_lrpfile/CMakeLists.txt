cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( shrader_lrpfile )

set( PROVIDER MS-Cheminformatics )
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets )
qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )
qtplatz_adplugin_install_dir( _adplugin_install_dir ${PROVIDER} )
set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

add_definitions(-DSHRADER_LRPFILE_LIBRARY)
#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Widgets ) # <-- adplugin depend on it

add_library( shrader_lrpfile SHARED
  shrader_lrpfile.cpp
  datafile_factory.cpp
  datafile_factory.hpp
  datafile.cpp datafile.hpp
  data_reader.cpp data_reader.hpp
)

target_link_libraries( shrader_lrpfile LINK_PUBLIC
  ${Boost_LIBRARIES}
  lrpfile
  adplugin
  adcontrols
  portfolio )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION "lib/qtplatz" COMPONENT libraries
)
