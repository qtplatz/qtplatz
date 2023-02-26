
project( Peptide )
set( PROVIDER MS-Cheminformatics )
if ( IDE_VERSION VERSION_LESS "9.0.0" )
  set( COMPAT_VERSION 3.2.81 )
endif()

set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
########## QtCreator 9 (Qt6) plugin.json #############
if ( ${IDE_VERSION} VERSION_GREATER_EQUAL "9.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package( QtCreator REQUIRED COMPONENTS Core )
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
  )
endif()
####################### END ##########################

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

set( peptide_SOURCES
  peptideplugin.cpp
  mainwindow.cpp
  peptidemode.cpp
  proteintable.cpp
  proteinwnd.cpp
  digestedpeptidetable.cpp
  )

set( peptide_HEADERS
  peptideplugin.hpp
  peptide_global.hpp
  peptideconstants.hpp
  mainwindow.hpp
  peptidemode.hpp
  proteintable.hpp
  proteinwnd.hpp
  digestedpeptidetable.hpp
  )

add_library( ${PROJECT_NAME} MODULE
  ${peptide_SOURCES}
  ${peptide_HEADERS}
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adplugin
  adcontrols
  adlog
  adutils
  adfs
  adprot
  adextension
  adplot
  adportable
  qtwrapper
  Core
  ExtensionSystem
  Utils
  Qt::Core
  ${Boost_LIBRARIES}
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
