cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( adnetcdf )

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

#include_directories( ${QtCore_INCLUDE_DIRS}  )
include_directories(
  ${QtWidgets_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src    # app_version.h
  )

add_definitions(-DADTXTFACTORY_LIBRARY)

# An issue need to be resolved
# several .cpp files are refered from adimport project

add_library( ${PROJECT_NAME} MODULE
  adnetcdf.cpp
  datafile.cpp
  datafile.hpp
  datafile_factory.cpp
  datafile_factory.hpp
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
