cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( chemistry )

set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Network Widgets Svg Sql PrintSupport )
find_package( rdkit )

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/MS-Chrminformatics ) #plugins

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec
  )

find_package( qwt )

add_definitions(
  -DDATAPROC_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${RDKIT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}  
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( chemistry_SOURCES
  chemistryplugin.cpp
  chemistrymode.cpp
  moltabledelegate.cpp
  chemfile.cpp
  mainwindow.cpp
  massdefectform.cpp
  massdefectdelegate.cpp
  massdefectmethod.cpp
  moltableview.cpp
  )

set( chemistry_HEADERS
  chemistryplugin.hpp
  chemistry_global.hpp
  moltabledelegate.hpp
  chemistryconstants.hpp
  chemistrymode.hpp
  chemfile.hpp
  mainwindow.hpp
  massdefectform.hpp
  massdefectdelegate.hpp
  massdefectmethod.hpp
  moltableview.hpp
  )

add_library( ${PROJECT_NAME} MODULE
  ${chemistry_SOURCES}
  ${chemistry_HEADERS}
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adutils
  adportable
  adlog
  adchem
  adcontrols
  adplugin
  qtwrapper
  adprot
  FileParsers
  GraphMol
  SmilesParse
  RDGeneral
  RDGeometryLib
  SubstructMatch
  Depictor
  Descriptors
  Core
  Qt5::Core
  Qt5::Svg
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

install( FILES
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )