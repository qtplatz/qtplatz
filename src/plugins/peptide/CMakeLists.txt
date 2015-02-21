
project( peptide )
set( PROVIDER MS-Cheminformatics )

cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

find_package( Qt5 CONFIG REQUIRED Core Network Widgets Sql Script )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set(COMPAT_VERSION 3.2.81)
set(VERSION 3.1.4)

configure_file(
  "${PROJECT_SOURCE_DIR}/pluginspec.in"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"  
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
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
  extensionsystem
  utils
  Qt5::Core
  ${Boost_LIBRARIES}
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
