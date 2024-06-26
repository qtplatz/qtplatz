cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( accutofacquire )

set( PROVIDER MS-Cheminformatics )

set( VERSION        "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
set( COMPAT_VERSION "${VERSION}" )
set( DESCRIPTION     "AccuTOF acquisition plugin" )

find_package( qwt )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

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
    "${CMAKE_SOURCE_DIR}/src/plugins/servant/plugin.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
    )
endif()
####################### END ##########################

if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
  configure_file(
    "${QTPLATZ_SOURCE_DIR}/src/plugins/pluginspec.in"
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    )
  execute_process( COMMAND ${XMLPATTERNS} -no-format
    -output ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json
    ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
    ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
    )
endif()

add_definitions(
  -DACCUTOFACQUIRE_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  -DBOOST_ASIO_NO_DEPRECATED
  -DXCHROMATOGRAMSMETHOD=1
  -DTOFCHROMATOGRAMSMETHOD=0
  )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/accutof/libs
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  ${QtCore_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src         # version.h
  )

find_package( Threads REQUIRED )

add_library(
  ${PROJECT_NAME} MODULE
  constants.cpp
  constants.hpp
  document.cpp
  document.hpp
  iu5303afacade.cpp
  iu5303afacade.hpp
  isequenceimpl.cpp
  mainwindow.cpp
  mainwindow.hpp
  pkdavgwriter.cpp
  pkdavgwriter.hpp
  resultwriter.cpp
  resultwriter.hpp
  task.cpp
  task.hpp
  mode.hpp
  mode.cpp
  accutofacquireplugin.cpp
  accutofacquireplugin.hpp
  waveformwnd.cpp
  waveformwnd.hpp
  isequenceimpl.cpp
  isequenceimpl.hpp
  moleculeswidget.cpp
  moleculeswidget.hpp
  ${Resources}
)

if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  set( EXTENSIONSYSTEM "ExtensionSystem" )
  set( UTILS "Utils" )
else()
  set( EXTENSIONSYSTEM "extensionsystem" )
  set( UTILS "utils" )
endif()

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  acewrapper
  acqrswidgets
  adacquire
  adacquire
  adcontrols
  adextension
  adfs
  adlog
  adplot
  adplugin
  adplugin_manager
  adportable
  adurl
  adutils
  adwidgets
  xmlparser
  Core
  ${EXTENSIONSYSTEM}
  ${UTILS}
  Qt::Svg
  Qt::Widgets
  acqrscontrols
  #  u5303a
  qtwrapper
  ${QWT_LIBRARIES}
  ${Boost_LIBRARIES}
  Threads::Threads
  )

if ( APPLE )
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}"
    )
else()
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_plugin_output_dir}"
    INSTALL_RPATH "\$ORIGIN:\$ORIGIN/..:\$ORIGIN/../..:\$ORIGIN/../../..:\$ORIGIN/../../../qtplatz:${CMAKE_INSTALL_RPATH}"
    )
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

if ( IDE_VERSION VERSION_LESS "8.0.0" )
  install( FILES
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
endif()
