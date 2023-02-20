#cmake_policy( SET CMP0020 NEW )
#cmake_policy( SET CMP0028 NEW )
project( servant )

set( PROVIDER MS-Cheminformatics )

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
if ( QT_VERSION_MAJOR GREATER_EQUAL 6 )
  find_package(QtCreator REQUIRED COMPONENTS Core)
endif()

set(CMAKE_AUTOMOC ON)

set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

########## QtCreator 9 (Qt6) plugin.json #############
if ( QT_VERSION_MAJOR GREATER_EQUAL 6 )
  set ( DEPENDENCY_STRING "\"Dependencies\" : [\n")
  foreach ( i "Core" )
    set(_v ${IDE_VERSION})
    string ( APPEND DEPENDENCY_STRING "        { \"Name\" : \"${i}\", \"Version\" : \"${_v}\" }" )
  endforeach()
  string(APPEND DEPENDENCY_STRING "\n    ]")

  configure_file(
    "${PROJECT_SOURCE_DIR}/plugin.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
    )
endif()
####################### END ##########################

if ( QT_VERSION_MAJOR LESS_EQUAL 5 )
  set(COMPAT_VERSION 3.2.81)
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    )

  execute_process( COMMAND ${XMLPATTERNS} -no-format
    -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
    ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  )

add_definitions( -DSERVANT_LIBRARY )

if ( ${QT_VERSION_MAJOR} GREATER 5 )
  set( EXTENSIONSYSTEM "ExtensionSystem" )
else()
  set( EXTENSIONSYSTEM "extensionsystem" )
endif()

add_library( servant MODULE
    outputwindow.cpp
    servant.cpp
    servantmode.cpp
    servantplugin.cpp
    logger.cpp
    servant_global.h
    outputwindow.hpp
    servant.hpp
    servantmode.hpp
    servantplugin.hpp
    logger.hpp
    )

  target_link_libraries( servant LINK_PUBLIC
    ${Boost_LIBRARIES}
    adcontrols
    adextension
    adportable
    adplugin
    adplugin_manager
    adlog
    qtwrapper
    adextension
    xmlparser
    Core
    ${EXTENSIONSYSTEM}
    Qt::Core
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

if ( ${QT_VERSION_MAJOR} LESS_EQUAL 5 )
  install( FILES
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
endif()
