cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( chemistry )

set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Network Widgets Svg Sql PrintSupport )
find_package( rdkit )
find_package( qwt )

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

execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec
  )

add_definitions( -DDATAPROC_LIBRARY  )

if ( MSVC )
  add_definitions(
    -D_SCL_SECURE_NO_WARNINGS
    ) 
endif()

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${RDKit_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}  
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( chemistry_HEADERS
  
  
  )

add_library( ${PROJECT_NAME} MODULE
  chemistryconstants.hpp
  chemistry_global.hpp
  chemistryplugin.cpp
  chemistryplugin.hpp
  chemconnection.cpp
  chemconnection.hpp
  chemquery.cpp
  chemquery.hpp
  chemschema.cpp
  chemschema.hpp
  chemfile.cpp
  chemfile.hpp
  document.cpp
  document.hpp
  mainwindow.cpp
  mainwindow.hpp
  mode.cpp
  mode.hpp
  massdefectform.cpp
  massdefectform.hpp
  massdefectdelegate.cpp
  massdefectdelegate.hpp
  massdefectmethod.cpp
  massdefectmethod.hpp
  moltablewnd.cpp
  moltablewnd.hpp
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adfs
  adutils
  adportable
  adlog
  adchem
  adcontrols
  adplugin
  adwidgets
  adprot
  qtwrapper
  Core
  ${RDKit_LIBRARIES}
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
