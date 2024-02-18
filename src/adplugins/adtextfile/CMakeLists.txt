cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( adtextfile )

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

#configure_file(
#  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.adplugin.in"
#  "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}/${PROJECT_NAME}.adplugin"
#)

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

##get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
##foreach(dir ${dirs})
##  message(STATUS "dir='${dir}'")
##endforeach()

add_definitions(-DADTXTFACTORY_LIBRARY)

# An issue need to be resolved
# several .cpp files are refered from adimport project

add_library( ${PROJECT_NAME} MODULE
  adtextfile.cpp
  adtextfile.hpp
  datafile.cpp
  datafile.hpp
  datafile_factory.cpp
  datafile_factory.hpp
  time_data_reader.cpp
  time_data_reader.hpp
  txtspectrum.cpp
  txtspectrum.hpp
  txtchromatogram.cpp
  txtchromatogram.hpp
  dialog.cpp
  dialog.hpp
  txt_reader.cpp
  txt_reader.hpp
  txt_tokenizer.cpp
  txt_tokenizer.hpp
)

include_directories(
  ${QtWidgets_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/${QTC_BINARY_DIR}/src/app
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${Boost_LIBRARIES}
  adcontrols
  adlog
  adportable
  xmlparser
  portfolio
  adplugin
  adfs
  Qt::Core
  Qt::Widgets
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
