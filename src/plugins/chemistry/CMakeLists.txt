#cmake_policy( SET CMP0020 NEW )
#cmake_policy( SET CMP0028 NEW )
if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
  project( chemistry )
else()
  project( Chemistry )
endif()

set( PROVIDER MS-Cheminformatics )

#find_package( rdkit )
#find_package( qwt )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

if ( QT_VERSION_MAJOR LESS_EQUAL 5 )
  set(COMPAT_VERSION 3.2.81)
endif()
set( DESCRIPTION "Chemistry plugin" )
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package( QtCreator REQUIRED COMPONENTS Core )

  set ( DEPENDENCY_STRING "\"Dependencies\" : [\n")
  foreach ( i "Core" )
    set(_v ${IDE_VERSION})
    string ( APPEND DEPENDENCY_STRING "        { \"Name\" : \"${i}\", \"Version\" : \"${_v}\" }" )
  endforeach()
  string(APPEND DEPENDENCY_STRING "\n    ]")

  configure_file(
    "${PROJECT_SOURCE_DIR}/../servant/plugin.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
    )
endif()
####################### END ##########################

if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    )
  execute_process( COMMAND ${XMLPATTERNS} -no-format
    -output ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json
    ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
    ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
    )
endif()

add_definitions( -DCHEMISTRY_LIBRARY )

if ( MSVC )
  add_definitions(
    -D_SCL_SECURE_NO_WARNINGS
    )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QWT_INCLUDE_DIRS}
  ${RDKit_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${QtSvg_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

#if ( QT_VERSION_MAJOR GREATER_EQUAL 6 )
#  include_directories( BEFORE
#    "${CMAKE_SOURCE_DIR}/qt-creator/src/libs"
#    "${CMAKE_SOURCE_DIR}/qt-creator/src/plugins"
#    )
#endif()

add_library( ${PROJECT_NAME} MODULE
  chemconnection.cpp
  chemconnection.hpp
  chemistryconstants.hpp
  chemistry_global.hpp
  chemistryplugin.cpp
  chemistryplugin.hpp
  chemquery.cpp
  chemquery.hpp
  chemschema.cpp
  chemschema.hpp
  chemspider.cpp
  chemspider.hpp
  document.cpp
  document.hpp
  mainwindow.cpp
  mainwindow.hpp
  mode.cpp
  mode.hpp
  moltablewnd.cpp
  moltablewnd.hpp
  queryform.cpp
  queryform.hpp
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adfs
  adurl
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
  Qt::Core
  Qt::Svg
  ${Boost_LIBRARIES}
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  LIBRARY DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
  install( FILES
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
endif()
