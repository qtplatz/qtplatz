cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( LipidId )

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( COMPAT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
set( DESCRIPTION "The LipidId plugin for QtCreator 9" )

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package( QtCreator REQUIRED COMPONENTS Core )
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
  )
endif()
####################### END ##########################

if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
  configure_file(
    "${CMAKE_SOURCE_DIR}/src/plugins/pluginspec.in"
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

  configure_file(
    "${PROJECT_SOURCE_DIR}/mimetype.xml"
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}-mimetype.xml"
  )

  execute_process( COMMAND ${XMLPATTERNS} -no-format
    -output ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json
    ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
    ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )
endif()

add_definitions(
  -D_SCL_SECURE_NO_WARNINGS
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}        # version.h
  ${CMAKE_SOURCE_DIR}/src    # app_version.h
  )

add_library( ${PROJECT_NAME} MODULE
  lipididplugin.cpp
  lipididplugin.hpp
  document.cpp
  document.hpp
  mainwindow.cpp
  mainwindow.hpp
  mode.cpp
  mode.hpp
  moltablewnd.cpp
  moltablewnd.hpp
  msspectrawnd.cpp
  msspectrawnd.hpp
  peaklist.cpp
  peaklist.hpp
  metidwidget.cpp
  metidwidget.hpp
  sdfimport.cpp
  sdfimport.hpp
  sqleditform.cpp
  sqleditform.hpp
  sqledit.cpp
  sqledit.hpp
  sqlexport.cpp sqlexport.hpp
  ionreactionwidget.cpp ionreactionwidget.hpp
  candidate.cpp
  candidate.hpp
  isocluster.cpp
  isocluster.hpp
  isopeak.cpp
  isopeak.hpp
  ionreaction.cpp ionreaction.hpp
  mol.cpp
  mol.hpp
  molgridwnd.cpp
  molgridwnd.hpp
  mspeaktree.cpp
  mspeaktree.hpp
  mspeakwidget.cpp
  mspeakwidget.hpp
  simple_mass_spectrum.cpp
  simple_mass_spectrum.hpp
  metidprocessor.cpp
  metidprocessor.hpp
  make_reference_spectrum.cpp
  make_reference_spectrum.hpp
  ${Resources}
  )

# for debian 10 suport
if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "8.3.0" )
  set (LIBCPPFS "stdc++fs")
endif()
message( STATUS "## LIBCPPFS: " ${LIBCPPFS} )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adextension
  adfs
  adlog
  adplot
  adplugin
  adportable
  adutils
  adwidgets
  qtwrapper
  ExtensionSystem
  Utils
  Core
  Qt::Core
  ${Boost_LIBRARIES}
  ${LIBCPPFS}
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
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}-mimetype.xml"
    DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
endif()
