#cmake_policy( SET CMP0020 NEW )
#cmake_policy( SET CMP0028 NEW )
project( Servant )

set( CMAKE_AUTOMOC ON )
set( PROVIDER MS-Cheminformatics )
set( DESCRIPTION "Servant plugin" )

set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

qtplatz_plugin_output_dir( _plugin_output_dir ${PROVIDER} )
qtplatz_plugin_install_dir( _plugin_install_dir ${PROVIDER} )

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package(QtCreator REQUIRED COMPONENTS Core)

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
  set(COMPAT_VERSION 3.2.81)
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    )

  execute_process( COMMAND ${XMLPATTERNS} -no-format
    -output ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json
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

if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  set( EXTENSIONSYSTEM "ExtensionSystem" )
else()
  set( EXTENSIONSYSTEM "extensionsystem" )
endif()

add_library( ${PROJECT_NAME} MODULE
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

  target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
    ${Boost_LIBRARIES}
    adcontrols
    adportable
    adplugin_manager
    adextension
    Core
    ${EXTENSIONSYSTEM}
    Qt::Core
  )

if ( APPLE )
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}"
    )
else()
  qtplatz_install_rpath( _rpath ${PROJECT_NAME} "${_plugin_install_dir}" )
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_plugin_output_dir}"
    INSTALL_RPATH "${_rpath}"
    )
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
